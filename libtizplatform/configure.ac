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
AC_INIT([tizplatform], [0.1.0], [juan.rubio@aratelia.com])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign color-tests silent-rules subdir-objects -Wall -Werror])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
LT_INIT

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG([0.25])

AC_DEFINE([HTTP_PARSER_STRICT], [1], [Using strict http parsing.])

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([libuuid], [uuid_generate])
AC_CHECK_LIB([ev], [ev_run])
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
PKG_CHECK_MODULES([UUID], [uuid >= 2.19.0])

AC_CHECK_HEADERS([tizonia/OMX_Core.h tizonia/OMX_Component.h],
	[tizrmd_found_omx_headers=yes; break;])
AS_IF([test "x$tizrmd_found_omx_headers" != "xyes"],
	[AC_SUBST([TIZILHEADERS_CFLAGS], ['-I$(top_srcdir)/../include/tizonia']),
	AC_SUBST([TIZILHEADERS_LIBS], ['not-used'])],
	[AC_MSG_NOTICE([Not substituting TIZILHEADERS cflags and libs with local paths])])
AS_IF([test "x$tizrmd_found_omx_headers" == "xyes"],
	[PKG_CHECK_MODULES([TIZILHEADERS], [tizilheaders >= 0.1.0])],
	[AC_MSG_NOTICE([Not using pkg-config to find TIZILHEADERS cflags and libs])])

AM_PATH_LOG4C([1.2.1])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# This one was introduced in 2.69
# AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero gettimeofday memmove memset pathconf socket strdup strerror strndup strstr strtoul])

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
   nw="$nw -Wsuggest-attribute=format" # Is it worth using attributes?
   nw="$nw -Wsuggest-attribute=noreturn" # Is it worth using attributes?
   #nw="$nw -Wsign-conversion"      # Likewise.
   # Enable all GCC warnings not in this list.
   gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
   for w in $warnings; do
     gl_WARN_ADD([$w])
     done

   gl_WARN_ADD([-fdiagnostics-show-option])
fi

AC_CONFIG_FILES([Makefile
                 libtizplatform.pc
                 src/Makefile
                 tests/Makefile])
AC_OUTPUT
