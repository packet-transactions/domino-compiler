AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AM_CPPFLAGS = -isystem /usr/lib/llvm-3.5/include/
AM_LDFLAGS = $(LLVM_LDFLAGS)
# There are some strange circular deps within clang
# https://github.com/eliben/llvm-clang-samples/blob/master/Makefile#L71
LDADD = -lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
				-lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
        -lLLVMOption  -lLLVMMCParser -lLLVMBitReader  -lLLVMMC -lLLVMSupport -lz -lpthread -ltinfo -ldl

common_source = clang_utility_functions.cc  expr_functions.cc clang_utility_functions.h expr_functions.h identifier_census.cc identifier_census.h unique_identifiers.h unique_identifiers.cc single_pass.h pkt_func_transform.h pkt_func_transform.cc graph.h cstr_array.h util.h file_descriptor.hh file_descriptor.cc temp_file.hh temp_file.cc exception.hh prog_transforms.h strength_reducer.cc expr_flattener_handler.cc expr_flattener_handler.h expr_propagater.cc stateful_flanks.cc ssa.cc partitioning.cc if_conversion_handler.cc if_conversion_handler.h ssa.h partitioning.h stateful_flanks.h

bin_PROGRAMS = domino

domino_SOURCES = $(common_source) domino.cc
