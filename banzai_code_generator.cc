#include "banzai_code_generator.h"

#include <string>

#include "clang/AST/Expr.h"
#include "clang/AST/Decl.h"

#include "clang_utility_functions.h"

using namespace clang;

const std::string BanzaiCodeGenerator::PACKET_IDENTIFIER = "packet";
const std::string BanzaiCodeGenerator::STATE_IDENTIFIER  = "state";

std::string BanzaiCodeGenerator::rewrite_into_banzai_ops(const clang::Stmt * stmt) const {
  assert(stmt);

  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += rewrite_into_banzai_ops(child) + ";";
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    std::string ret;
    ret += "if (" + rewrite_into_banzai_ops(if_stmt->getCond()) + ") {" + rewrite_into_banzai_ops(if_stmt->getThen()) + " }";
    if (if_stmt->getElse() != nullptr) {
      ret += "else {" + rewrite_into_banzai_ops(if_stmt->getElse()) + " }";
    }
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return rewrite_into_banzai_ops(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + rewrite_into_banzai_ops(bin_op->getRHS());
  } else if (isa<MemberExpr>(stmt)) {
    const auto * member_expr = dyn_cast<MemberExpr>(stmt);
    // All packet fields are of the type p(...) in banzai
    // N.B. the Banzai code overloads the () operator.
    return   PACKET_IDENTIFIER + "(\"" + clang_value_decl_printer(member_expr->getMemberDecl()) + "\")";
  } else if (isa<DeclRefExpr>(stmt)) {
    // All state variables are of the type s(...) in banzai
    // N.B. Again by overloading the () operator
    const auto * decl_expr = dyn_cast<DeclRefExpr>(stmt);
    return   STATE_IDENTIFIER + "(\"" + clang_value_decl_printer(decl_expr->getDecl()) + "\")";
  } else {
    throw std::logic_error("rewrite_into_banzai_ops cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}

std::string BanzaiCodeGenerator::rewrite_into_banzai_atom(const clang::Stmt * stmt)  const {
  return "Packet " +
         unique_identifiers_.get_unique_identifier("atom") +
         "(const Packet & " + PACKET_IDENTIFIER + ", const State & " + STATE_IDENTIFIER + " __attribute__((unused))) { " +
         rewrite_into_banzai_ops(stmt) +
         " return " + PACKET_IDENTIFIER + "; }";
}
