#include <iostream>
#include "clang/AST/AST.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "single_pass.h"
#include "clang_utility_functions.h"

using namespace clang;
using namespace clang::tooling;

static auto echo_transform(const TranslationUnitDecl * tu_decl) { return clang_decl_printer(tu_decl); }

static llvm::cl::OptionCategory echo_pass(""
"Driver program for single pass program that takes"
"in a file and echoes it out as such");


int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, echo_pass);

  // Parse file once and output it as such. This is just a test program
  std::cout << SinglePass<std::string>(op, echo_transform).output();
}
