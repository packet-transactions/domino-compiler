AM_CPPFLAGS =  -isystem /usr/lib/llvm-3.5/include
AM_CXXFLAGS = $(PICKY_CXXFLAGS) -isystem $(srcdir)/../third_party/ -pthread
LDADD = $(srcdir)/../third_party/libgtest.a

# Define unit tests
TESTS = echo_test
check_PROGRAMS = $(TESTS)

# domino tests
echo_test_SOURCES = echo_test.cc
