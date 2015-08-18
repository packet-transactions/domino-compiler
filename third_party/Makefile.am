AM_CXXFLAGS = $(PICKY_CXXFLAGS)

noinst_LIBRARIES = libmahimahi.a libgtest.a
libmahimahi_a_SOURCES = exception.hh file_descriptor.hh file_descriptor.cc temp_file.hh temp_file.cc child_process.hh child_process.cc system_runner.hh system_runner.cc util.cc util.hh signalfd.hh signalfd.cc assert_exception.hh

# Remove our picky C flags to compile gtest
libgtest_a_SOURCES = gtest-all.cpp gtest.h gtest-main.cc
libgtest_a_CXXFLAGS =
