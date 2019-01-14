ACLOCAL_AMFLAGS = -Iexternal/m4
@ifGNUmake@ include $(srcdir)/build-aux/am/aminclude_static.am
AM_CPPFLAGS = -I./build-aux/h
# Leave CONFDIR a command-line argument: sysconfdir must be recursively
# expanded by make, which AC_DEFINE does not do.
AM_CPPFLAGS += -DLOCALSTATEDIR=\"$(localstatedir)\"
AM_CXXFLAGS = -Wall -Wextra -Wpedantic -Werror

# Binary compilation
bin_PROGRAMS = rngminder
rngminder_SOURCES = src/main.c

# Test execution
AX_GNU_AUTOTEST_CHECK_FLAGS = AUTOTEST_PATH=.
@AX_GNU_AUTOTEST_DEFAULT@

# Valgrind
VALGRIND_FLAGS = --quiet
VALGRIND_CHECKTARGET = check-autotest
@VALGRIND_CHECK_RULES@
MOSTLYCLEANFILES = tests/test-suite-drd.log\
 tests/test-suite-helgrind.log\
 tests/test-suite-memcheck.log\
 tests/test-suite-sgcheck.log\
 -r\
 tests/test-suite-drd.log.dir\
 tests/test-suite-helgrind.log.dir\
 tests/test-suite-memcheck.log.dir\
 tests/test-suite-sgcheck.log.dir

# Code coverage
CODE_COVERAGE_OUTPUT_FILE = coverage.info
CODE_COVERAGE_OUTPUT_DIRECTORY = coverage
CODE_COVERAGE_BRANCH_COVERAGE = 1
CODE_COVERAGE_GENHTML_OPTIONS = $(CODE_COVERAGE_GENHTML_OPTIONS_DEFAULT)\
 --prefix "`readlink -f $(abs_srcdir)`"
CODE_COVERAGE_IGNORE_PATTERN = "/usr/include/*"\
 "`readlink -f $(abs_srcdir)`/tests/*"
AM_CPPFLAGS += $(CODE_COVERAGE_CPPFLAGS)
AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)
AM_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)

# Documentation
@DX_RULES@
MOSTLYCLEANFILES += $(DX_CLEANFILES)

# Distrubution
EXTRA_DIST = .gitignore\
 autogen.sh\
 LICENSE\
 README.md
