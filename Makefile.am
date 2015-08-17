SUBDIRS = third_party .

AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AM_CPPFLAGS = -isystem /usr/lib/llvm-3.5/include/
AM_LDFLAGS = $(LLVM_LDFLAGS)
# There are some strange circular deps within clang requiring us to list libraries twice
# https://github.com/eliben/llvm-clang-samples/blob/master/Makefile#L71
CLANG_LLVM_LIBS = -lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
				-lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
        -lLLVMOption  -lLLVMMCParser -lLLVMBitReader  -lLLVMMC -lLLVMSupport -lz -lpthread -ltinfo -ldl

common_source = clang_utility_functions.cc  expr_functions.cc clang_utility_functions.h expr_functions.h unique_identifiers.h unique_identifiers.cc pkt_func_transform.h pkt_func_transform.cc graph.h strength_reducer.cc expr_flattener_handler.cc expr_flattener_handler.h expr_prop.cc stateful_flanks.cc ssa.cc partitioning.cc if_conversion_handler.cc if_conversion_handler.h ssa.h partitioning.h stateful_flanks.h expr_prop.h strength_reducer.h banzai_code_generator.h banzai_code_generator.cc util.cc util.h banzai_atom.h banzai_atom.cc assert_exception.h

bin_PROGRAMS = domino banzai_code_gen_test

domino_SOURCES = $(common_source) domino.cc
domino_LDADD = $(srcdir)/third_party/libmahimahi.a $(CLANG_LLVM_LIBS)

banzai_code_gen_test_SOURCES = $(common_source) banzai_code_gen_test.cc
banzai_code_gen_test_LDADD = $(srcdir)/third_party/libmahimahi.a $(CLANG_LLVM_LIBS)
