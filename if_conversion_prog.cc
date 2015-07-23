#include <iostream>
#include <string>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "state_var_decl_handler.h"
#include "packet_decl_handler.h"
#include "if_conversion_handler.h"
#include "run_pass.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory if_conversion(""
"This allows us to rewrite if statements into ternary operators"
" and recursively get rid of all branches, from the innermost to"
" the outermost ones.");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, if_conversion);

  // Run passes, chaining results if required
  const auto state_vars     = run_pass<StateVarDeclHandler, std::string>(op);
  const auto prog_decl_pair = run_pass<IfConversionHandler, std::pair<std::string, std::vector<std::string>>>(op);
  const auto packet_decls   = run_pass<PacketDeclHandler, std::string>(op, prog_decl_pair.second);

  // Print out outputs in sequence
  std::cout << state_vars << std::endl
            << packet_decls << std::endl
            << prog_decl_pair.first << std::endl;

  return 0;
}
