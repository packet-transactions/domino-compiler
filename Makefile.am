AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AM_CPPFLAGS = -isystem /usr/lib/llvm-3.5/include/
AM_LDFLAGS = $(LLVM_LDFLAGS)
LDADD = libclangall.a $(LLVM_LIBS) $(LLVM_SYSLIBS)
common_source = clang_utility_functions.cc  expr_functions.cc clang_utility_functions.h   expr_functions.h  

bin_PROGRAMS = partitioning_prog if_conversion_prog struct_to_local_vars
partitioning_prog_SOURCES = $(common_source) partitioning_handler.cc  partitioning_prog.cc partitioning_handler.h 
if_conversion_prog_SOURCES = $(common_source) if_conversion_prog.cc if_conversion_handler.cc if_conversion_handler.h
struct_to_local_vars_SOURCES = $(common_source) struct_to_local_vars.cc
