#include <algorithm>
#include <iterator>
#include "clang_utility_functions.h"
#include "expr_functions.h"

using namespace clang;

std::set<std::string> ExprFunctions::get_all_vars(const clang::Expr * expr) {
  // Get all stateful right hand sides of inst
  assert(expr);
  if (isa<ParenExpr>(expr)) {
    return get_all_vars(dyn_cast<ParenExpr>(expr)->getSubExpr());
  } else if (isa<CastExpr>(expr)) {
    return get_all_vars(dyn_cast<CastExpr>(expr)->getSubExpr());
  } else if (isa<UnaryOperator>(expr)) {
    return get_all_vars(dyn_cast<UnaryOperator>(expr)->getSubExpr());
  } else if (isa<ConditionalOperator>(expr)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(expr);

    // Get condition, true , and false expressions
    auto cond_set = get_all_vars(cond_op->getCond());
    auto true_set = get_all_vars(cond_op->getTrueExpr());
    auto false_set = get_all_vars(cond_op->getFalseExpr());

    // Union together true and false sets
    std::set<std::string> value_set;
    std::set_union(true_set.begin(), true_set.end(),
                   false_set.begin(), false_set.end(),
                   std::inserter(value_set, value_set.begin()));

    // Union that with cond_set
    std::set<std::string> ret;
    std::set_union(value_set.begin(), value_set.end(),
                   cond_set.begin(), cond_set.end(),
                   std::inserter(ret, ret.begin()));
    return ret;
  } else if (isa<BinaryOperator>(expr)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(expr);
    auto lhs_set = get_all_vars(bin_op->getLHS());
    auto rhs_set = get_all_vars(bin_op->getRHS());
    std::set<std::string> ret;
    std::set_union(lhs_set.begin(), lhs_set.end(),
                   rhs_set.begin(), rhs_set.end(),
                   std::inserter(ret, ret.begin()));
    return ret;
  } else if (isa<DeclRefExpr>(expr)) {
    const auto read_var = clang_stmt_printer(dyn_cast<DeclRefExpr>(expr));
    if (read_var.find("__") == std::string::npos) {
      return std::set<std::string>({read_var});
    }
    return std::set<std::string>();
  } else {
    assert(isa<IntegerLiteral>(expr));
    return std::set<std::string>();
  }
}
