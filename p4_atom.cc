#include <iostream>

#include "p4_atom.h"

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"

using namespace clang;
const std::string P4Atom::PACKET_IDENTIFIER = "packet";
const std::string P4Atom::STATE_SCALAR_IDENTIFIER  = "scalar";
const std::string P4Atom::STATE_ARRAY_IDENTIFIER = "array";

P4Atom::P4Atom(const clang::Stmt * stmt,
                       const std::string & t_name)
    : field_list_str_(""),
      name_(t_name),
      function_body_(rewrite_into_p4_ops(stmt)),
      atom_definition_(field_list_str_ + "action " + name_ + "() {" + function_body_ + "}\n\n")
{}

std::string P4Atom::rewrite_into_p4_ops(const clang::Stmt * stmt) {
  assert_exception(stmt);
  static int field_list_count = 0;
  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += rewrite_into_p4_ops(child) + ";";
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    std::string ret;
    ret += "if (" + rewrite_into_p4_ops(if_stmt->getCond()) + ") {" + rewrite_into_p4_ops(if_stmt->getThen()) + "; }";
    if (if_stmt->getElse() != nullptr) {
      ret += "else {" + rewrite_into_p4_ops(if_stmt->getElse()) + "; }";
    }
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    std::string ret;
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);

    if (bin_op->isAssignmentOp()) {
      // Now get RHS and LHS (assigned var)
      const auto * rhs = bin_op->getRHS();
      const auto * assigned_var = bin_op->getLHS();

      // If we have a modulo in the expression, it must be a hash function
      if (isa<BinaryOperator>(rhs) and std::string(dyn_cast<BinaryOperator>(rhs)->getOpcodeStr()) == "%") {
        // Call to a hash function
        if (isa<CallExpr>(dyn_cast<BinaryOperator>(rhs)->getLHS())) {
          auto id = field_list_count++;
          field_list_str_ = "field_list hash_fields_" + std::to_string(id) + "{\n";
          const auto * call_expr = dyn_cast<CallExpr>(dyn_cast<BinaryOperator>(rhs)->getLHS());
          assert_exception(call_expr);
          for (const auto * child : call_expr->arguments()) {
            const auto child_str = rewrite_into_p4_ops(child);
            field_list_str_ += child_str + ";\n";
          }
          field_list_str_ += "}\n";
          field_list_str_ += "field_list_calculation hash_" + std::to_string(id) + "{ input { hash_fields_" + std::to_string(id) + "; } algorithm : crc16; output_width : 32;}\n";
          ret = "modify_field_with_hash_based_offset(" + clang_stmt_printer(assigned_var) + ", 0, hash_" + std::to_string(id) + ", " + clang_stmt_printer(dyn_cast<BinaryOperator>(rhs)->getRHS())+")";
          return ret;
        } else {
          std::cout << "rhs is " << clang_stmt_printer(rhs) << "\n";
          throw std::logic_error("rewrite_into_p4_ops cannot handle stmt " + clang_stmt_printer(stmt) + " of type " + std::string(stmt->getStmtClassName()));
        }
      } else {
         return rewrite_into_p4_ops(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + rewrite_into_p4_ops(bin_op->getRHS());
      }
    } else {
      return rewrite_into_p4_ops(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + rewrite_into_p4_ops(bin_op->getRHS());
    }
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return     rewrite_into_p4_ops(cond_op->getCond()) + " ? "
             + rewrite_into_p4_ops(cond_op->getTrueExpr()) + " : "
             + rewrite_into_p4_ops(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<DeclRefExpr>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + rewrite_into_p4_ops(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + rewrite_into_p4_ops(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return rewrite_into_p4_ops(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    // TODO: Handle this specially for a P4 backend
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = rewrite_into_p4_ops(child);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else if (isa<NullStmt>(stmt)) {
    return "";
  } else {
    throw std::logic_error("rewrite_into_p4_ops cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
