#include <iostream>
#include "clang/Tooling/CommonOptionsParser.h"
#include "single_pass.h"
#include "echo_transform.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory echo_pass(""
"Driver program for single pass program that takes"
"in a file and echoes it out as such");


int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, echo_pass);

  // Parse file once and output it
  std::cout << SinglePass<std::string>(op, echo_transform).output();
}
