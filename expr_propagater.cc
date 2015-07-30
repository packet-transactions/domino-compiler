#include <iostream>
#include <set>
#include <string>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "run_pass.h"
#include "func_transform_handler.cc"
#include "packet_decl_creator.h"
#include "state_var_decl_handler.h"
#include "expr_propagation_handler.h"
#include "func_decl_pass_through.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory strength_redux(""
"Simple strength reduction: rewrite if (1) ? x : y to x."
"Simplify 1 && x to x");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, strength_redux);

  // Run passes, chaining results if required
  const auto state_vars     = run_pass<StateVarDeclHandler,
                                      std::string>
                                      (op, clang::ast_matchers::decl().bind("decl"));

  const auto func_decls     = run_pass<FuncDeclPassThrough,
                                      std::string>
                                      (op, clang::ast_matchers::decl().bind("decl"));


  const auto packet_decls   = run_pass<PacketDeclCreator,
                                      std::string>
                                      (op, clang::ast_matchers::decl().bind("decl"),
                                      std::vector<std::string>());

  const auto strength_redux = run_pass<FuncTransformHandler<ExprPropagationHandler>,
                                      std::pair<std::string, std::vector<std::string>>>
                                      (op, clang::ast_matchers::functionDecl().bind("functionDecl"));
  // Print out outputs in sequence
  std::cout << state_vars << std::endl
            << packet_decls << std::endl
            << func_decls << std::endl
            << strength_redux.first << std::endl;

  return 0;
}
