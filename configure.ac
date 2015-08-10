AC_PREREQ([2.61])
AC_INIT([domino], [0.1], [anirudh@csail.mit.edu])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([ssa.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX

# Add picky CXXFLAGS
CPPFLAGS="-std=c++14 -pthread"
PICKY_CXXFLAGS="-pedantic -Wconversion -Wsign-conversion -Wall -Wextra -Weffc++ -Werror -fno-default-inline"
AC_SUBST([PICKY_CXXFLAGS])

# Add llvm ldflags
LLVM_LDFLAGS=`llvm-config --ldflags`
AC_SUBST([LLVM_LDFLAGS])

# Checks for header files.
AC_LANG_PUSH(C++)

AC_CHECK_HEADERS([algorithm array cassert cmath queue \
cstdio string sys/stat.h sys/types.h ctime tuple unistd.h unordered_map \
utility vector], [], [AC_MSG_ERROR([Missing header file])])

# Check for clang
AC_PATH_PROG([CLANG], [clang], [])
AS_IF([test x"$CLANG" = x],
[AC_MSG_ERROR([cannot find clang])])

# C++ libraries are harder to check (http://nerdland.net/2009/07/detecting-c-libraries-with-autotools/),
# so use headers to check
AC_CHECK_HEADER([llvm/Pass.h], [], [AC_MSG_ERROR([LLVM headers not found])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_LANG_POP(C++)

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
