#include "expr_flattener_handler.h"

#include <iostream>

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace clang;
using std::placeholders::_1;
using std::placeholders::_2;

std::string ExprFlattenerHandler::transform(const TranslationUnitDecl * tu_decl) {
  unique_identifiers_ = UniqueIdentifiers(identifier_census(tu_decl));
  return pkt_func_transform(tu_decl, std::bind(& ExprFlattenerHandler::flatten_body, this, _1, _2));
}

std::pair<std::string, std::vector<std::string>>
ExprFlattenerHandler::flatten_body(const Stmt * function_body, const std::string & pkt_name) const {
  assert_exception(function_body);

  std::string output = "";
  std::vector<std::string> new_decls = {};

  // iterate through function body
  assert_exception(isa<CompoundStmt>(function_body));
  for (const auto & child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    const auto ret = flatten(bin_op->getRHS(), pkt_name);

    // First add new definitions
    output += ret.new_defs;

    // Then add flattened expression
    output += clang_stmt_printer(bin_op->getLHS()) + " = " + ret.flat_expr + ";";

    // Then append new pkt var declarations
    new_decls.insert(new_decls.end(), ret.new_decls.begin(), ret.new_decls.end());
  }

  return make_pair("{" + output + "}", new_decls);
}

bool ExprFlattenerHandler::is_atomic_expr(const clang::Expr * expr) const {
  expr = expr->IgnoreParenImpCasts();
  return isa<DeclRefExpr>(expr) or isa<IntegerLiteral>(expr) or isa<MemberExpr>(expr) or isa<CallExpr>(expr) or isa<ArraySubscriptExpr>(expr);
}

bool ExprFlattenerHandler::is_flat(const clang::Expr * expr) const {
  expr = expr->IgnoreParenImpCasts();
  assert_exception(expr);
  if (isa<UnaryOperator>(expr)) {
    return is_atomic_expr(dyn_cast<UnaryOperator>(expr)->getSubExpr());
  } else if (isa<ConditionalOperator>(expr)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(expr);
    return is_atomic_expr(cond_op->getCond()) and is_atomic_expr(cond_op->getTrueExpr()) and is_atomic_expr(cond_op->getFalseExpr());
  } else if (isa<BinaryOperator>(expr)) {
    return is_atomic_expr(dyn_cast<BinaryOperator>(expr)->getLHS()) and
           is_atomic_expr(dyn_cast<BinaryOperator>(expr)->getRHS());
  } else {
    assert_exception(is_atomic_expr(expr));
    return true;
  }
}

FlattenResult ExprFlattenerHandler::flatten(const clang::Expr * expr, const std::string & pkt_name) const {
  expr = expr->IgnoreParenImpCasts();
  if (is_flat(expr)) {
    return {clang_stmt_printer(expr), "", {}};
  } else {
    if (isa<ConditionalOperator>(expr)) {
      return flatten_cond_op(dyn_cast<ConditionalOperator>(expr), pkt_name);
    } else if (isa<BinaryOperator>(expr)) {
      return flatten_bin_op(dyn_cast<BinaryOperator>(expr), pkt_name);
    } else {
      assert_exception(false);
      return {"", "", {}};
    }
  }
}

FlattenResult ExprFlattenerHandler::flatten_bin_op(const BinaryOperator * bin_op, const std::string & pkt_name) const {
  assert_exception(not is_flat(bin_op));
  const auto ret_lhs = flatten_to_atomic_expr(bin_op->getLHS(), pkt_name);
  const auto ret_rhs = flatten_to_atomic_expr(bin_op->getRHS(), pkt_name);

  // Join all declarations
  std::vector<std::string> all_decls;
  all_decls.insert(all_decls.begin(), ret_lhs.new_decls.begin(), ret_lhs.new_decls.end());
  all_decls.insert(all_decls.begin(), ret_rhs.new_decls.begin(), ret_rhs.new_decls.end());

  return {ret_lhs.flat_expr + std::string(BinaryOperator::getOpcodeStr(bin_op->getOpcode())) + ret_rhs.flat_expr,
          ret_lhs.new_defs + ret_rhs.new_defs,
          all_decls};
}

FlattenResult ExprFlattenerHandler::flatten_cond_op(const ConditionalOperator * cond_op, const std::string & pkt_name) const {
  assert_exception(not is_flat(cond_op));
  const auto ret_cond  = flatten_to_atomic_expr(cond_op->getCond(), pkt_name);
  const auto ret_true  = flatten_to_atomic_expr(cond_op->getTrueExpr(), pkt_name);
  const auto ret_false = flatten_to_atomic_expr(cond_op->getFalseExpr(), pkt_name);

  // Join all declarations
  std::vector<std::string> all_decls;
  all_decls.insert(all_decls.begin(), ret_cond.new_decls.begin(), ret_cond.new_decls.end());
  all_decls.insert(all_decls.begin(), ret_true.new_decls.begin(), ret_true.new_decls.end());
  all_decls.insert(all_decls.begin(), ret_false.new_decls.begin(), ret_false.new_decls.end());

  return {ret_cond.flat_expr + " ? " + ret_true.flat_expr + " : " + ret_false.flat_expr,
          ret_cond.new_defs + ret_true.new_defs + ret_false.new_defs,
          all_decls};
}

FlattenResult ExprFlattenerHandler::flatten_to_atomic_expr(const Expr * expr, const std::string & pkt_name) const {
  if (is_atomic_expr(expr)) {
    return {clang_stmt_printer(expr), "", {}};
  } else {
    const auto flat_var_member     = unique_identifiers_.get_unique_identifier();
    const auto flat_var_decl       = expr->getType().getAsString() + " " + flat_var_member + ";";
    const auto pkt_flat_variable   = pkt_name + "." + flat_var_member;
    const auto pkt_flat_var_def    = pkt_flat_variable + " = " + clang_stmt_printer(expr) + ";";
    return {pkt_flat_variable, pkt_flat_var_def, {flat_var_decl}};
  }
}
