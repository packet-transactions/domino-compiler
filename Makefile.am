SUBDIRS = third_party . tests

AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AM_CPPFLAGS = -isystem /usr/lib/llvm-3.5/include/
AM_LDFLAGS = $(LLVM_LDFLAGS)
# There are some strange circular deps within clang requiring us to list libraries twice
# https://github.com/eliben/llvm-clang-samples/blob/master/Makefile#L71
CLANG_LLVM_LIBS = -lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
				-lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
        -lLLVMOption  -lLLVMMCParser -lLLVMBitReader  -lLLVMMC -lLLVMSupport -lz -lpthread -ltinfo -ldl

common_source = clang_utility_functions.cc  clang_utility_functions.h unique_identifiers.h unique_identifiers.cc pkt_func_transform.h pkt_func_transform.cc graph.h strength_reducer.cc expr_flattener_handler.cc expr_flattener_handler.h expr_prop.cc stateful_flanks.cc ssa.cc partitioning.cc if_conversion_handler.cc if_conversion_handler.h ssa.h partitioning.h stateful_flanks.h expr_prop.h strength_reducer.h banzai_code_generator.h banzai_code_generator.cc util.cc util.h banzai_atom.h banzai_atom.cc assert_exception.h desugar_compound_assignment.h desugar_compound_assignment.cc int_type_checker.h int_type_checker.cc algebraic_simplifier.cc algebraic_simplifier.h array_validator.cc array_validator.h redundancy_remover.cc redundancy_remover.h sketch_backend.cc sketch_backend.h csi.cc csi.h cse.cc cse.h

noinst_LIBRARIES = libdomino.a
bin_PROGRAMS = domino

domino_SOURCES = domino.cc
domino_LDADD = $(CLANG_LLVM_LIBS) libdomino.a $(srcdir)/third_party/libmahimahi.a
libdomino_a_SOURCES = $(common_source)
