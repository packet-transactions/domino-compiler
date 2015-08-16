#include "banzai_atom.h"

#include "clang/AST/Expr.h"

#include "clang_utility_functions.h"

using namespace clang;
const std::string BanzaiAtom::PACKET_IDENTIFIER = "packet";
const std::string BanzaiAtom::STATE_IDENTIFIER  = "state";

BanzaiAtom::BanzaiAtom(const clang::Stmt * stmt, const std::string & t_name, const std::map<std::string, uint32_t> & state_initializers)
    : name_(t_name),
      function_body_(rewrite_into_banzai_ops(stmt)),
      function_definition_("[] (Packet & " + PACKET_IDENTIFIER + " __attribute__((unused)), State & " + STATE_IDENTIFIER + " __attribute__((unused))) {\n" +
                           function_body_ + "\n }"),
      state_vars_used_(gen_var_list(stmt, VariableType::STATE)),
      atom_definition_("Atom " + name_ + "(" + function_definition_ + ", " + state_init_string(state_vars_used_, state_initializers) + ");")
{}

std::string BanzaiAtom::state_init_string(const std::set<std::string> & state_vars_used, const std::map<std::string, uint32_t> & state_initializers) const {
  // Generate initial values for all state variables used within stmt
  std::string init_state_str = "FieldContainer(std::map<FieldContainer::FieldName, uint32_t>";
  init_state_str += "{";
  for (const auto & state_var : state_vars_used) {
    init_state_str += "{\"" + state_var + "\", " + std::to_string(state_initializers.at(state_var)) + "},";
  }
  init_state_str += '}'; // to close std::map constructor's initializer list
  init_state_str += ")"; // to close FieldContainer constructor's left parenthesis
  return init_state_str;
}

std::string BanzaiAtom::rewrite_into_banzai_ops(const clang::Stmt * stmt) const {
  assert(stmt);

  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += rewrite_into_banzai_ops(child) + ";";
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    std::string ret;
    ret += "if (" + rewrite_into_banzai_ops(if_stmt->getCond()) + ") {" + rewrite_into_banzai_ops(if_stmt->getThen()) + "; }";
    if (if_stmt->getElse() != nullptr) {
      ret += "else {" + rewrite_into_banzai_ops(if_stmt->getElse()) + "; }";
    }
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return rewrite_into_banzai_ops(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + rewrite_into_banzai_ops(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return   "(" + rewrite_into_banzai_ops(cond_op->getCond()) + ") ? ("
             + rewrite_into_banzai_ops(cond_op->getTrueExpr()) + ") : ("
             + rewrite_into_banzai_ops(cond_op->getFalseExpr()) + ")";
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
