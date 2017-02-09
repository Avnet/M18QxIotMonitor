AC_PREREQ(2.61)
AC_INIT([host-mode-demo], 1.0.0)
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([main.cpp])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_PROG_CXX
PKG_PROG_PKG_CONFIG

# Checks for libraries.

AC_ARG_WITH([common_includes],
      AC_HELP_STRING([--with-common-includes=@<:@dir@:>@],
         [Specify the location of the common headers]),
      [common_incdir=$withval],
      with_common_includes=no)

if test "x$with_common_includes" != "xno"; then
   CFLAGS="${CFLAGS} -I${common_incdir}"
fi

AC_SUBST([CFLAGS])
AC_SUBST([CPPFLAGS])

AC_CONFIG_FILES([ \
        Makefile
        ])
AC_OUTPUT
