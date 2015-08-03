#include <iostream>
#include <set>
#include <string>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "run_pass.h"
#include "expr_flattener_handler.h"
#include "packet_variable_census.h"
#include "pkt_func_transform.h"
#include "single_pass.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory expr_flattener(""
"Flatten expressions using temporaries so that every"
"statement is of the form x = y op z;");

int main(int argc, const char **argv) {
  // Set up parser options
  CommonOptionsParser op(argc, argv, expr_flattener);

  // Run passes, chaining results if required
  const auto packet_var_set       = run_pass<PacketVariableCensus,
                                            std::set<std::string>>
                                            (op, clang::ast_matchers::decl().bind("decl"));

  ExprFlattenerHandler expr_flattener_handler(packet_var_set);
  // Parse file once and output it
  const FuncBodyTransform expr_flattener = std::bind(& ExprFlattenerHandler::transform, expr_flattener_handler, std::placeholders::_1, std::placeholders::_2);
  std::cout << SinglePass<std::string>(op, std::bind(pkt_func_transform, std::placeholders::_1, expr_flattener)).output();

  return 0;
}
