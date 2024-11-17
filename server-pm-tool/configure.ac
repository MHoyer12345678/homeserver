dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([server-pm-tool],[1.0])

AM_SILENT_RULES([yes])

LT_INIT
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_CXX

PKG_CHECK_MODULES([GPIOD],[libgpiod >= 1.6.0])

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
