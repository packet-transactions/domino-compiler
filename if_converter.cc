#include <iostream>
#include <set>
#include <string>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "run_pass.h"
#include "packet_variable_census.h"
#include "if_conversion_handler.h"
#include "pkt_func_transform.h"
#include "single_pass.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory if_conversion(""
"This allows us to rewrite if statements into ternary operators"
" and recursively get rid of all branches, from the innermost to"
" the outermost ones.");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, if_conversion);

  // Run passes, chaining results if required
  const auto packet_var_set = run_pass<PacketVariableCensus,
                                      std::set<std::string>>
                                      (op, clang::ast_matchers::decl().bind("decl"));

  IfConversionHandler if_conversion_handler(packet_var_set);
  // Parse file once and output it
  const FuncBodyTransform if_converter = std::bind(& IfConversionHandler::transform, if_conversion_handler, std::placeholders::_1, std::placeholders::_2);
  std::cout << SinglePass<std::string>(op, std::bind(pkt_func_transform, std::placeholders::_1, if_converter)).output();

  return 0;
}
