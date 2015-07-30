#include <iostream>
#include "clang_utility_functions.h"
#include "expr_propagation_handler.h"

using namespace clang;

std::pair<std::string, std::vector<std::string>> ExprPropagationHandler::transform(const Stmt * function_body, const std::string & pkt_name __attribute__((unused))) const {
  assert(function_body);

  std::string output = "";
  std::vector<std::string> new_decls = {};

  // Maintain map from variable name (packet or state variable)
  // to a string representing its expression, for expression propagation.
  std::map<std::string, std::string> var_to_expr;

  // iterate through function body
  assert(isa<CompoundStmt>(function_body));
  for (const auto & child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());

    // Strip off parenthesis and casts for LHS and RHS
    const auto * rhs = bin_op->getRHS()->IgnoreParenImpCasts();
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();

    // Populate var_to_expr
    var_to_expr[clang_stmt_printer(lhs)] = clang_stmt_printer(rhs);

    if ((isa<DeclRefExpr>(rhs) or isa<MemberExpr>(rhs)) and (var_to_expr.find(clang_stmt_printer(rhs)) != var_to_expr.end())) {
      // If rhs is a packet/state variable, replace it with its current expr
      output += clang_stmt_printer(lhs) + "=" + var_to_expr.at(clang_stmt_printer(rhs)) + ";";
    } else {
      // Pass through
      output += clang_stmt_printer(lhs) + "=" + clang_stmt_printer(rhs) + ";";
    }
  }

  return make_pair(output, new_decls);
}
