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
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return   rewrite_into_banzai_ops(cond_op->getCond()) + " ? "
           + rewrite_into_banzai_ops(cond_op->getTrueExpr()) + " : "
           + rewrite_into_banzai_ops(cond_op->getFalseExpr()) + " ;";
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
  } else if (isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + rewrite_into_banzai_ops(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return rewrite_into_banzai_ops(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else {
    throw std::logic_error("rewrite_into_banzai_ops cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}

std::string BanzaiCodeGenerator::rewrite_into_banzai_atom(const clang::Stmt * stmt)  const {
  return "void " +
         unique_identifiers_.get_unique_identifier("atom") +
         "(Packet & " + PACKET_IDENTIFIER + ", State & " + STATE_IDENTIFIER + " __attribute__((unused))) {\n" +
         rewrite_into_banzai_ops(stmt) + "\n }";
}

std::string BanzaiCodeGenerator::transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const {
  // Storage for returned string
  std::string ret;

  for (const auto * child_decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    assert(child_decl);
    if (isa<VarDecl>(child_decl) or
        (isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) or
        isa<RecordDecl>(child_decl)) {
      // Just quench these, don't emit them
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      ret = rewrite_into_banzai_atom(dyn_cast<FunctionDecl>(child_decl)->getBody());
    } else {
      assert(isa<TypedefDecl>(child_decl));
    }
  }
  return ret;
}
