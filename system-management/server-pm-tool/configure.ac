dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([server-pm-tool],[1.0])

AM_SILENT_RULES([yes])

LT_INIT
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_CXX

# Derive path for storing systemd service files (e. g. /lib/systemd/system)
AC_ARG_WITH([systemdsystemunitdir],
        AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files]),
        [],
        [with_systemdsystemunitdir=$(pkg-config --silence-errors --variable=systemdsystemunitdir systemd)])

if test "x${with_systemdsystemunitdir}" = "x"; then
            with_systemdsystemunitdir="/lib/systemd/system"
fi

AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
AS_IF([test "x$with_systemdsystemunitdir" != "xno"],
      [AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])])
      
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemdsystemunitdir" != "xno"])

PKG_CHECK_MODULES([GPIOD],[libgpiod >= 1.6.0])

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
