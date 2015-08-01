#include <iostream>
#include <set>
#include <string>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "single_pass.h"
#include "strength_reducer_transform.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory strength_redux(""
"Simple strength reduction: rewrite if (1) ? x : y to x."
"Simplify 1 && x to x");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, strength_redux);

  // Parse file once and output it
  std::cout << SinglePass(op, strength_reducer_transform).output();

  return 0;
}
