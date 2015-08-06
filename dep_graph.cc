#include <iostream>
#include "clang/AST/AST.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "single_pass.h"
#include "clang_utility_functions.h"

using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory dep_graph(""
"Print out dependency graph of the program as a dot file");

int main(int argc, const char ** argv) {
  // Parser options
  CommonOptionsParser op(argc, argv, dep_graph);

  // Parse file once and output dot file
  std::cout << SinglePass<std::string>(op, clang_decl_printer).output();
}
