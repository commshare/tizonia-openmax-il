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
AC_INIT([tizcore], [0.1.0], [juan.rubio@aratelia.com])
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
AC_CHECK_LIB([dl], [dlopen])
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

AC_CHECK_HEADERS([tizonia/OMX_Core.h tizonia/OMX_Component.h],
	[tizcore_found_omx_headers=yes; break;])
AS_IF([test "x$tizcore_found_omx_headers" != "xyes"],
	[AC_SUBST([TIZILHEADERS_CFLAGS], ['-I$(top_srcdir)/../include/tizonia']),
	AC_SUBST([TIZILHEADERS_LIBS], ['not-used'])],
	[AC_MSG_NOTICE([Not substituting TIZILHEADERS cflags and libs with local paths])])
AS_IF([test "x$tizcore_found_omx_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZILHEADERS], [tizilheaders >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZILHEADERS cflags and libs])])

AC_CHECK_HEADERS([tizonia/tizplatform.h],
	[tizcore_found_platform_headers=yes; break;])
AS_IF([test "x$tizcore_found_platform_headers" != "xyes"],
	[AC_SUBST([TIZPLATFORM_CFLAGS], ['-I$(top_srcdir)/../libtizplatform/tizonia']),
	AC_SUBST([TIZPLATFORM_LIBS], ['$(top_builddir)/../libtizplatform/tizonia/libtizplatform.la'])],
	[AC_MSG_NOTICE([Not substituting TIZPLATFORM cflags and libs with local paths])])
AS_IF([test "x$tizcore_found_platform_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZPLATFORM], [libtizplatform >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZPLATFORM cflags and libs])])

AC_CHECK_HEADERS([tizonia/tizrmproxytypes.h],
	[tizcore_found_rmproxy_headers=yes; break;])
AS_IF([test "x$tizcore_found_rmproxy_headers" != "xyes"],
	[AC_SUBST([TIZRMPROXY_CFLAGS], ['-I$(top_srcdir)/../rm/libtizrmproxy/tizonia']),
	AC_SUBST([TIZRMPROXY_LIBS], ['$(top_builddir)/../rm/libtizrmproxy/tizonia/libtizrmproxy.la'])],
	[AC_MSG_NOTICE([Not substituting TIZRMPROXY cflags and libs with local paths])])
AS_IF([test "x$tizcore_found_rmproxy_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZRMPROXY], [libtizrmproxy >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZRMPROXY cflags and libs])])

AC_CHECK_HEADERS([tizonia/tizrmtypes.h],
	[tizcore_found_rmd_headers=yes; break;])
AS_IF([test "x$tizcore_found_rmd_headers" != "xyes"],
	[AC_SUBST([TIZRMD_CFLAGS], ['-I$(top_srcdir)/../rm/tizrmd/tizonia']),
	AC_SUBST([TIZRMD_LIBS], ['not-used'])],
	[AC_MSG_NOTICE([Not substituting TIZRMD cflags and libs with local paths])])
AS_IF([test "x$tizcore_found_rmd_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZRMD], [tizrmd >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZRMD cflags and libs])])


# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# This one was introduced in 2.69
# AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
# TODO: Move some of these to libtizplatform
AC_FUNC_FORK
AC_CHECK_FUNCS([bzero strerror strndup strstr])

# Additional GCC warnings option
AC_ARG_ENABLE([gcc-warnings],
[AS_HELP_STRING([--enable-gcc-warnings],
[turn on lots of GCC warnings (for developers)])],
[case $enableval in
yes|no) ;;
*) AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
esac
gl_gcc_warnings=$enableval],
[gl_gcc_warnings=no]
)

if test "$gl_gcc_warnings" = yes; then
   gl_MANYWARN_ALL_GCC([warnings])
   # Set up the list of the pointless, undesired warnings.
   nw=
   nw="$nw -Wsystem-headers"       # Don't let system headers trigger warnings
   nw="$nw -Wundef"                # All compiler preprocessors support #if UNDEF
   nw="$nw -Wtraditional"          # All compilers nowadays support ANSI C
   # nw="$nw -Wconversion"           # These warnings usually don't point to mistakes.
   # nw="$nw -Wsign-conversion"      # Likewise.
   nw="$nw -Wsuggest-attribute=pure"  # Is it worth using attributes?
   nw="$nw -Wsuggest-attribute=const" # Is it worth using attributes?
   # Enable all GCC warnings not in this list.
   gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
   for w in $warnings; do
     gl_WARN_ADD([$w])
     done

   gl_WARN_ADD([-fdiagnostics-show-option])
fi

AC_CONFIG_FILES([Makefile
                 libtizcore.pc
                 src/Makefile
                 test_component/Makefile
                 tests/Makefile])

# End the configure script.
AC_OUTPUT
