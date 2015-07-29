#include <iostream>
#include "clang/AST/Expr.h"
#include "clang_utility_functions.h"
#include "expr_flattener_handler.h"

using namespace clang;

std::pair<std::string, std::vector<std::string>>
ExprFlattenerHandler::transform(const Stmt * function_body, const std::string & pkt_name) const {
  assert(function_body);

  std::string output = "";
  std::vector<std::string> new_decls = {};

  // iterate through function body
  assert(isa<CompoundStmt>(function_body));
  for (const auto & child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());
    const auto ret = flatten(bin_op->getRHS(), pkt_name);

    // First add new definitions
    output += ret.new_defs;

    // Then add flattened expression
    output += clang_stmt_printer(bin_op->getLHS()) + " = " + ret.flat_expr + ";";

    // Then append new pkt var declarations
    new_decls.insert(new_decls.begin(), ret.new_decls.begin(), ret.new_decls.end());
  }

  return make_pair(output, new_decls);
}

bool ExprFlattenerHandler::is_atom(const clang::Expr * expr) const {
  expr = expr->IgnoreParenImpCasts();
  return isa<DeclRefExpr>(expr) or isa<IntegerLiteral>(expr) or isa<MemberExpr>(expr);
}

bool ExprFlattenerHandler::is_flat(const clang::Expr * expr) const {
  expr = expr->IgnoreParenImpCasts();
  assert(expr);
  if (isa<UnaryOperator>(expr)) {
    return is_atom(dyn_cast<UnaryOperator>(expr)->getSubExpr());
  } else if (isa<ConditionalOperator>(expr)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(expr);
    return is_atom(cond_op->getCond()) and is_atom(cond_op->getTrueExpr()) and is_atom(cond_op->getFalseExpr());
  } else if (isa<BinaryOperator>(expr)) {
    return is_atom(dyn_cast<BinaryOperator>(expr)->getLHS()) and
           is_atom(dyn_cast<BinaryOperator>(expr)->getRHS());
  } else {
    assert(is_atom(expr));
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
      assert(false);
      return {"", "", {}};
    }
  }
}

FlattenResult ExprFlattenerHandler::flatten_bin_op(const BinaryOperator * bin_op, const std::string & pkt_name) const {
  assert(not is_flat(bin_op));
  const auto ret_lhs = flatten_to_atom(bin_op->getLHS(), pkt_name);
  const auto ret_rhs = flatten_to_atom(bin_op->getRHS(), pkt_name);

  // Join all declarations
  std::vector<std::string> all_decls;
  all_decls.insert(all_decls.begin(), ret_lhs.new_decls.begin(), ret_lhs.new_decls.end());
  all_decls.insert(all_decls.begin(), ret_rhs.new_decls.begin(), ret_rhs.new_decls.end());

  return {ret_lhs.flat_expr + std::string(BinaryOperator::getOpcodeStr(bin_op->getOpcode())) + ret_rhs.flat_expr,
          ret_lhs.new_defs + ret_rhs.new_defs,
          all_decls};
}

FlattenResult ExprFlattenerHandler::flatten_cond_op(const ConditionalOperator * cond_op, const std::string & pkt_name) const {
  assert(not is_flat(cond_op));
  const auto ret_cond  = flatten_to_atom(cond_op->getCond(), pkt_name);
  const auto ret_true  = flatten_to_atom(cond_op->getTrueExpr(), pkt_name);
  const auto ret_false = flatten_to_atom(cond_op->getFalseExpr(), pkt_name);

  // Join all declarations
  std::vector<std::string> all_decls;
  all_decls.insert(all_decls.begin(), ret_cond.new_decls.begin(), ret_cond.new_decls.end());
  all_decls.insert(all_decls.begin(), ret_true.new_decls.begin(), ret_true.new_decls.end());
  all_decls.insert(all_decls.begin(), ret_false.new_decls.begin(), ret_false.new_decls.end());

  return {ret_cond.flat_expr + " ? " + ret_true.flat_expr + " : " + ret_false.flat_expr,
          ret_cond.new_defs + ret_true.new_defs + ret_false.new_defs,
          all_decls};
}

FlattenResult ExprFlattenerHandler::flatten_to_atom(const Expr * expr, const std::string & pkt_name) const {
  if (is_atom(expr)) {
    return {clang_stmt_printer(expr), "", {}};
  } else {
    const auto flat_var_member     = get_unique_var();
    const auto flat_var_decl       = "int " + flat_var_member + ";";
    const auto pkt_flat_variable   = pkt_name + "." + flat_var_member;
    const auto pkt_flat_var_def    = pkt_flat_variable + " = " + clang_stmt_printer(expr) + ";";
    return {pkt_flat_variable, pkt_flat_var_def, {flat_var_decl}};
  }
}

std::string ExprFlattenerHandler::get_unique_var() const {
  // Propose tmp_x, where x starts at var_suffix_ + 1
  // and keeps incrementing until at least some tmp_x
  // is unique and does not belong to var_set_
  var_suffix_ = var_suffix_ + 1;
  std::string candidate = "tmp" + std::to_string(var_suffix_);
  while (var_set_.find(candidate) != var_set_.end()) {
    var_suffix_++;
    candidate = "tmp" + std::to_string(var_suffix_);
  }
  assert(var_set_.find(candidate) == var_set_.end());
  var_set_.emplace(candidate);
  return candidate;
}
