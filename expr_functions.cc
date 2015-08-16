#include "expr_functions.h"

#include <algorithm>
#include <iterator>

#include "clang_utility_functions.h"
#include "set_idioms.h"

using namespace clang;

std::string ExprFunctions::replace_vars(const clang::Expr * expr,
                                        const std::map<std::string, std::string> & repl_map) {
  assert(expr);
  if (isa<ParenExpr>(expr)) {
    return replace_vars(dyn_cast<ParenExpr>(expr)->getSubExpr(), repl_map);
  } else if (isa<CastExpr>(expr)) {
    return replace_vars(dyn_cast<CastExpr>(expr)->getSubExpr(), repl_map);
  } else if (isa<UnaryOperator>(expr)) {
    const auto * un_op = dyn_cast<UnaryOperator>(expr);
    assert(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert(opcode_str == "!");
    return opcode_str + replace_vars(un_op->getSubExpr(), repl_map);
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
