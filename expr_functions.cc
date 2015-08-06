#include <algorithm>
#include <iterator>
#include "clang_utility_functions.h"
#include "expr_functions.h"

using namespace clang;

std::set<std::string> ExprFunctions::get_vars(const clang::Expr * expr) {
  // Get all stateful right hand sides of inst
  assert(expr);
  if (isa<ParenExpr>(expr)) {
    return get_vars(dyn_cast<ParenExpr>(expr)->getSubExpr());
  } else if (isa<CastExpr>(expr)) {
    return get_vars(dyn_cast<CastExpr>(expr)->getSubExpr());
  } else if (isa<UnaryOperator>(expr)) {
    return get_vars(dyn_cast<UnaryOperator>(expr)->getSubExpr());
  } else if (isa<ConditionalOperator>(expr)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(expr);

    // Get condition, true , and false expressions
    auto cond_set = get_vars(cond_op->getCond());
    auto true_set = get_vars(cond_op->getTrueExpr());
    auto false_set = get_vars(cond_op->getFalseExpr());

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
    auto lhs_set = get_vars(bin_op->getLHS());
    auto rhs_set = get_vars(bin_op->getRHS());
    std::set<std::string> ret;
    std::set_union(lhs_set.begin(), lhs_set.end(),
                   rhs_set.begin(), rhs_set.end(),
                   std::inserter(ret, ret.begin()));
    return ret;
  } else if (isa<DeclRefExpr>(expr)) {
    // All DeclRefExpr are stateful variables
    const auto read_var = clang_stmt_printer(dyn_cast<DeclRefExpr>(expr));
    return std::set<std::string>({read_var});
  } else if (isa<MemberExpr>(expr)) {
    // All MemberExpr are packet variables
    return std::set<std::string>({clang_stmt_printer(dyn_cast<MemberExpr>(expr))});
  } else if (isa<CallExpr>(expr)) {
    const auto * call_expr = dyn_cast<CallExpr>(expr);
    std::set<std::string> ret;
    for (const auto * child : call_expr->arguments()) {
      const auto child_uses = get_vars(child);
      std::set_union(child_uses.begin(), child_uses.end(),
                     ret.begin(), ret.end(),
                     std::inserter(ret, ret.begin()));
    }
    return ret;
  } else {
    assert(isa<IntegerLiteral>(expr));
    return std::set<std::string>();
  }
}

std::string ExprFunctions::replace_vars(const clang::Expr * expr,
                                        const std::map<std::string, std::string> & repl_map) {
  // Get all stateful right hand sides of inst
  assert(expr);
  if (isa<ParenExpr>(expr)) {
    return replace_vars(dyn_cast<ParenExpr>(expr)->getSubExpr(), repl_map);
  } else if (isa<CastExpr>(expr)) {
    return replace_vars(dyn_cast<CastExpr>(expr)->getSubExpr(), repl_map);
  } else if (isa<UnaryOperator>(expr)) {
    return replace_vars(dyn_cast<UnaryOperator>(expr)->getSubExpr(), repl_map);
  } else if (isa<ConditionalOperator>(expr)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(expr);

    const auto cond_str = replace_vars(cond_op->getCond(), repl_map);
    const auto true_str = replace_vars(cond_op->getTrueExpr(), repl_map);
    const auto false_str = replace_vars(cond_op->getFalseExpr(), repl_map);

    return "(" + cond_str + ") ? (" + true_str + ") : (" + false_str + ")";
  } else if (isa<BinaryOperator>(expr)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(expr);
    const auto lhs_str = replace_vars(bin_op->getLHS(), repl_map);
    const auto rhs_str = replace_vars(bin_op->getRHS(), repl_map);
    return lhs_str + std::string(BinaryOperator::getOpcodeStr(bin_op->getOpcode())) + rhs_str;
  } else if (isa<DeclRefExpr>(expr)) {
    // All DeclRefExpr are stateful variables
    const std::string state_var_name = clang_stmt_printer(expr);
    if (repl_map.find(state_var_name) != repl_map.end()) {
      return repl_map.at(state_var_name);
    } else {
      return state_var_name;
    }
  } else if (isa<MemberExpr>(expr)) {
    // All MemberExpr are packet variables
    const std::string pkt_var_name = clang_stmt_printer(expr);
    if (repl_map.find(pkt_var_name) != repl_map.end()) {
      return repl_map.at(pkt_var_name);
    } else {
      return pkt_var_name;
    }
  } else if (isa<CallExpr>(expr)) {
    const auto * call_expr = dyn_cast<CallExpr>(expr);
    std::string ret = "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = replace_vars(child, repl_map);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else {
    assert(isa<IntegerLiteral>(expr));
    return clang_stmt_printer(expr);
  }
}
