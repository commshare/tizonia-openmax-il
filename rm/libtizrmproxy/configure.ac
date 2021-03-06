# Copyright (C) 2011-2015 Aratelia Limited - Juan A. Rubio
#
# This file is part of Tizonia
#
# Tizonia is free software: you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# Tizonia is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Tizonia.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.67])
AC_INIT([tizrmproxy], [0.1.0], [juan.rubio@aratelia.com])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign color-tests silent-rules -Wall -Werror])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# 'm4' is the directory where the extra autoconf macros are stored
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
LT_INIT
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG()

# Checks for libraries.
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
DBUS_REQUIRED_VERSION=0.60
PKG_CHECK_MODULES(dbus, [dbus-1 >= $DBUS_REQUIRED_VERSION],,
	AC_MSG_ERROR([You need the DBus libraries (version 0.6 or better)]
	[http://www.freedesktop.org/wiki/Software_2fdbus]))
PKG_CHECK_MODULES([DBUS], [dbus-c++-1 >= 0.6.0-pre1])

AC_CHECK_HEADERS([tizonia/OMX_Core.h tizonia/OMX_Component.h],
	[tizrmproxy_found_omx_headers=yes; break;])
AS_IF([test "x$tizrmproxy_found_omx_headers" != "xyes"],
	[AC_SUBST([TIZILHEADERS_CFLAGS], ['-I$(top_srcdir)/../../include/tizonia']),
	AC_SUBST([TIZILHEADERS_LIBS], ['not-used'])],
	[AC_MSG_NOTICE([Not substituting TIZILHEADERS cflags and libs with local paths])])
AS_IF([test "x$tizrmproxy_found_omx_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZILHEADERS], [tizilheaders >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZILHEADERS cflags and libs])])

AC_CHECK_HEADERS([tizonia/tizplatform.h],
	[tizrmproxy_found_platform_headers=yes; break;])
AS_IF([test "x$tizrmproxy_found_platform_headers" != "xyes"],
	[AC_SUBST([TIZPLATFORM_CFLAGS], ['-I$(top_srcdir)/../../libtizplatform/tizonia']),
	AC_SUBST([TIZPLATFORM_LIBS], ['$(top_builddir)/../../libtizplatform/tizonia/libtizplatform.la'])],
	[AC_MSG_NOTICE([Not substituting TIZPLATFORM cflags and libs with local paths])])
AS_IF([test "x$tizrmproxy_found_platform_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZPLATFORM], [libtizplatform >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZPLATFORM cflags and libs])])

AC_CHECK_HEADERS([tizonia/tizrmtypes.h],
	[tizrmproxy_found_rmd_headers=yes; break;])
AS_IF([test "x$tizrmproxy_found_rmd_headers" != "xyes"],
	[AC_SUBST([TIZRMD_CFLAGS], ['-I$(top_srcdir)/../tizrmd/tizonia']),
	AC_SUBST([TIZRMD_LIBS], ['not-used'])],
	[AC_MSG_NOTICE([Not substituting TIZRMD cflags and libs with local paths])])
AS_IF([test "x$tizrmproxy_found_rmd_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZRMD], [tizrmd >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZRMD cflags and libs])])

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# This one was introduced in 2.69
# AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([strndup strstr strtol])

# DBus
AS_AC_EXPAND(DATADIR, $datadir)
DBUS_SERVICES_DIR="$DATADIR/dbus-1/services"
AC_SUBST(DBUS_SERVICES_DIR)
AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR",
                                      [Where services dir for DBUS is])

AC_CONFIG_FILES([Makefile
                libtizrmproxy.pc
                src/Makefile
                tests/Makefile])

# End the configure script.
AC_OUTPUT
