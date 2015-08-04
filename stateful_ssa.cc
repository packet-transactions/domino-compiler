#include <iostream>
#include <set>
#include <string>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang_utility_functions.h"
#include "packet_variable_census.h"
#include "pkt_func_transform.h"
#include "single_pass.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory stateful_ssa(""
"Stateful SSA form. This is our intermediate representation"
"where we have a read preamble where all state variables are read"
"into temporary variables. Then the rest of the program operates"
"on these temporary variables and is written in SSA form: no temporary"
"variable is ever assigned twice. We close the program with a write"
"postamble that takes temporary variables and writes them into state"
"variables again");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, stateful_ssa);

  // Parse file once and generate set of all packet variables
  const auto packet_var_set = SinglePass<std::set<std::string>>(op, packet_variable_census).output();

  // Parse file once and output stateful ssa form
  std::cout << SinglePass<std::string>(op, clang_decl_printer).output();
}
