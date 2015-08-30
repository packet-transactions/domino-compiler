AM_CPPFLAGS =  -isystem /usr/lib/llvm-3.5/include
AM_CXXFLAGS = $(PICKY_CXXFLAGS) -isystem $(srcdir)/../third_party/ -pthread
AM_LDFLAGS = $(LLVM_LDFLAGS)

# There are some strange circular deps within clang requiring us to list libraries twice
# https://github.com/eliben/llvm-clang-samples/blob/master/Makefile#L71
CLANG_LLVM_LIBS = -lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
				-lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
        -lLLVMOption  -lLLVMMCParser -lLLVMBitReader  -lLLVMMC -lLLVMSupport -lz -lpthread -ltinfo -ldl

LDADD = $(CLANG_LLVM_LIBS) $(srcdir)/../third_party/libgtest.a $(srcdir)/../libdomino.a $(srcdir)/../third_party/libmahimahi.a

# Define unit tests
TESTS = echo_test if_converter_test strength_reducer_test expr_flattener_test scc_test
check_PROGRAMS = $(TESTS)

# domino tests
echo_test_SOURCES = echo_test.cc
if_converter_test_SOURCES = if_converter_test.cc
strength_reducer_test_SOURCES = strength_reducer_test.cc
expr_flattener_test_SOURCES = expr_flattener_test.cc
scc_test_SOURCES = scc_test.cc
