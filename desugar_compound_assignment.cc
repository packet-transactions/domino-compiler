#include "desugar_compound_assignment.h"

#include <iostream>

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace clang;

std::string desugar_compound_assignment_transform(const TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, desugar_compound_assignment_helper);
}

std::pair<std::string, std::vector<std::string>> desugar_compound_assignment_helper(const clang::CompoundStmt * body,
                                                                                    const std::string & pkt_name __attribute__((unused))) {
  return std::make_pair("{" + desugar_compound_assignments(body) + "}", std::vector<std::string>());
}

BinaryOperator::Opcode get_underlying_op(const BinaryOperator::Opcode & comp_asgn_op) {
  // This is a clang quirk:
  // opcodes for all operators with compound assignment variants
  // aren't contiguous.
  if (comp_asgn_op >= BinaryOperator::Opcode::BO_MulAssign and
      comp_asgn_op <= BinaryOperator::Opcode::BO_ShrAssign) {
    return static_cast<BinaryOperator::Opcode>(static_cast<int>(comp_asgn_op) - 19);
  } else if (comp_asgn_op >= BinaryOperator::Opcode::BO_AndAssign and
             comp_asgn_op <= BinaryOperator::Opcode::BO_OrAssign) {
    return static_cast<BinaryOperator::Opcode>(static_cast<int>(comp_asgn_op) - 13);
  } else {
    throw std::logic_error("Got opcode that get_underlying_op can't handle: " + std::string(BinaryOperator::getOpcodeStr(comp_asgn_op)));
  }
}

std::string desugar_compound_assignments(const Stmt * stmt) {
  assert_exception(stmt);
  std::string ret;
  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += desugar_compound_assignments(child) + ";";
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    std::string ret;
    ret += "if (" + desugar_compound_assignments(if_stmt->getCond()) + ") {" + desugar_compound_assignments(if_stmt->getThen()) + "; }";
    if (if_stmt->getElse() != nullptr) {
      ret += "else {" + desugar_compound_assignments(if_stmt->getElse()) + "; }";
    }
    return ret;
  } else if (isa<CompoundAssignOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return       desugar_compound_assignments(bin_op->getLHS())
           + "=" + desugar_compound_assignments(bin_op->getLHS())
           + std::string(BinaryOperator::getOpcodeStr(get_underlying_op(bin_op->getOpcode())))
           + desugar_compound_assignments(bin_op->getRHS());
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return desugar_compound_assignments(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + desugar_compound_assignments(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return     desugar_compound_assignments(cond_op->getCond()) + " ? "
             + desugar_compound_assignments(cond_op->getTrueExpr()) + " : "
             + desugar_compound_assignments(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt) or isa<DeclRefExpr>(stmt) or isa<ArraySubscriptExpr>(stmt) or isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + desugar_compound_assignments(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + desugar_compound_assignments(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return desugar_compound_assignments(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = desugar_compound_assignments(child);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else {
    throw std::logic_error("desugar_compound_assignments cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
