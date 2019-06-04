AM_CXXFLAGS = $(PICKY_CXXFLAGS) -isystem $(srcdir)/../third_party/
AM_CPPFLAGS = -isystem $(CLANG_DEV_LIBS)/include/
AM_LDFLAGS = -L $(CLANG_DEV_LIBS)/lib

# There are some strange circular deps within clang requiring us to list libraries twice
# https://github.com/eliben/llvm-clang-samples/blob/master/Makefile#L71
CLANG_LLVM_LIBS = -lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
                  -lclangAST -lclangAnalysis -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool -lclangLex -lclangParse -lclangSema -lclangSerialization\
                  -lLLVMCore -lLLVMOption  -lLLVMMCParser -lLLVMBitReader  -lLLVMMC -lLLVMDemangle -lLLVMProfileData -lLLVMSupport -lLLVMBinaryFormat -lz -lpthread -ltermcap -ldl

LDADD = $(CLANG_LLVM_LIBS) $(srcdir)/../third_party/libgtest.a $(srcdir)/../libdomino.a $(srcdir)/../third_party/libmahimahi.a -lpthread

# Define unit tests
TESTS = echo_test if_converter_test expr_flattener_test scc_test complicated_scc_test
check_PROGRAMS = $(TESTS)

# domino tests
echo_test_SOURCES = echo_test.cc
echo_test_CXXFLAGS = $(AM_CXXFLAGS) -fno-rtti
if_converter_test_SOURCES = if_converter_test.cc
if_converter_test_CXXFLAGS = $(AM_CXXFLAGS) -fno-rtti
expr_flattener_test_SOURCES = expr_flattener_test.cc
expr_flattener_test_CXXFLAGS = $(AM_CXXFLAGS) -fno-rtti
scc_test_SOURCES = scc_test.cc
scc_test_CXXFLAGS = $(AM_CXXFLAGS) -fno-rtti
complicated_scc_test_SOURCES = complicated_scc_test.cc
complicated_scc_test_CXXFLAGS = $(AM_CXXFLAGS) -fno-rtti
