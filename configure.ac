#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rngminder], [0.1.0-dev], [software@menlosystems.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux/sh])
AC_CONFIG_HEADERS([build-aux/h/config.h])
AC_DEFUN([AMINCLUDE_STATIC], [build-aux/am/aminclude_static.am])
AM_INIT_AUTOMAKE([-Wall -Werror foreign nostdinc subdir-objects tar-ustar])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h\
 stdlib.h\
 string.h\
 sys/ioctl.h\
 unistd.h], ,
    [AC_MSG_ERROR("cannot find system include file")])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Post-build checks.
AX_GNU_AUTOTEST
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK
AX_CODE_COVERAGE

# Documentation.
DX_DOT_FEATURE([ON])
DX_PDF_FEATURE([OFF])
DX_PS_FEATURE([OFF])
DX_INIT_DOXYGEN(m4_defn([AC_PACKAGE_NAME]), [$(srcdir)/doc/Doxyfile])

# Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
