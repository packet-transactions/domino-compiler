#include "ast_visitor.h"

#include <iostream>
#include <functional>

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace std::placeholders;
using namespace clang;

std::string AstVisitor::ast_visit_transform(const TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, std::bind(&AstVisitor::ast_visit_helper, this, _1, _2));
}

std::pair<std::string, std::vector<std::string>> AstVisitor::ast_visit_helper(const clang::CompoundStmt * body,
                                                                              const std::string & pkt_name __attribute__((unused))) {
  return std::make_pair("{" + ast_visit_stmt(body) + "}", std::vector<std::string>());
}

std::string AstVisitor::ast_visit_comp_stmt(const CompoundStmt * comp_stmt) {
  assert_exception(comp_stmt);
  std::string ret;
  for (const auto & child : comp_stmt->children())
    ret += ast_visit_stmt(child) + ";";
  return ret;
}

std::string AstVisitor::ast_visit_if_stmt(const IfStmt * if_stmt) {
  assert_exception(if_stmt);
  std::string ret;
  ret += "if (" + ast_visit_stmt(if_stmt->getCond()) + ") {" + ast_visit_stmt(if_stmt->getThen()) + "}";
  if (if_stmt->getElse() != nullptr) {
    ret += "else {" + ast_visit_stmt(if_stmt->getElse()) + "}";
  }
  return ret;
}

std::string AstVisitor::ast_visit_bin_op(const BinaryOperator * bin_op) {
  assert_exception(bin_op);
  return ast_visit_stmt(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + ast_visit_stmt(bin_op->getRHS());
}

std::string AstVisitor::ast_visit_cond_op(const ConditionalOperator * cond_op) {
  assert_exception(cond_op);
  return     ast_visit_stmt(cond_op->getCond()) + " ? "
             + ast_visit_stmt(cond_op->getTrueExpr()) + " : "
             + ast_visit_stmt(cond_op->getFalseExpr());
}

std::string AstVisitor::ast_visit_func_call(const CallExpr * call_expr) {
  assert_exception(call_expr);
  std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
  for (const auto * child : call_expr->arguments()) {
    const auto child_str = ast_visit_stmt(child);
    ret += child_str + ",";
  }
  ret.back() = ')';
  return ret;
}

std::string AstVisitor::ast_visit_member_expr(const MemberExpr * member_expr) {
  assert_exception(member_expr);
  return clang_stmt_printer(member_expr);
}

std::string AstVisitor::ast_visit_decl_ref_expr(const DeclRefExpr * decl_ref_expr) {
  assert_exception(decl_ref_expr);
  return clang_stmt_printer(decl_ref_expr);
}

std::string AstVisitor::ast_visit_array_subscript_expr(const ArraySubscriptExpr * array_subscript_expr) {
  assert_exception(array_subscript_expr);
  return clang_stmt_printer(array_subscript_expr);
}

std::string AstVisitor::ast_visit_integer_literal(const IntegerLiteral * integer_literal) {
  assert_exception(integer_literal);
  return clang_stmt_printer(integer_literal);
}

std::string AstVisitor::ast_visit_un_op(const UnaryOperator * un_op) {
  assert_exception(un_op->isArithmeticOp());
  const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
  assert_exception(opcode_str == "!");
  return opcode_str + ast_visit_stmt(un_op->getSubExpr());
}

std::string AstVisitor::ast_visit_implicit_cast(const ImplicitCastExpr * implicit_cast) {
  assert_exception(implicit_cast);
  return ast_visit_stmt(implicit_cast->getSubExpr());
}

std::string AstVisitor::ast_visit_stmt(const Stmt * stmt) {
  assert_exception(stmt);
  std::string ret;
  if(isa<CompoundStmt>(stmt)) {
    return ast_visit_comp_stmt(dyn_cast<CompoundStmt>(stmt));
  } else if (isa<IfStmt>(stmt)) {
    return ast_visit_if_stmt(dyn_cast<IfStmt>(stmt));
  } else if (isa<BinaryOperator>(stmt)) {
    return ast_visit_bin_op(dyn_cast<BinaryOperator>(stmt));
  } else if (isa<ConditionalOperator>(stmt)) {
    return ast_visit_cond_op(dyn_cast<ConditionalOperator>(stmt));
  } else if (isa<MemberExpr>(stmt)) {
    return ast_visit_member_expr(dyn_cast<MemberExpr>(stmt));
  } else if (isa<DeclRefExpr>(stmt)) {
    return ast_visit_decl_ref_expr(dyn_cast<DeclRefExpr>(stmt));
  } else if (isa<IntegerLiteral>(stmt)) {
    return ast_visit_integer_literal(dyn_cast<IntegerLiteral>(stmt));
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    return ast_visit_array_subscript_expr(dyn_cast<ArraySubscriptExpr>(stmt));
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + ast_visit_stmt(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    return ast_visit_un_op(dyn_cast<UnaryOperator>(stmt));
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return ast_visit_implicit_cast(dyn_cast<ImplicitCastExpr>(stmt));
  } else if (isa<CallExpr>(stmt)) {
    return ast_visit_func_call(dyn_cast<CallExpr>(stmt));
  } else if (isa<NullStmt>(stmt)) {
    return "";
  } else {
    throw std::logic_error("ast_visit error: the statement\n"
                           + clang_stmt_printer(stmt)
                           + "\nis of type "
                           + std::string(stmt->getStmtClassName())
                           + ", which isn't allowed in domino");
  }
}
