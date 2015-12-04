#include "pisa_atom.h"

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"

using namespace clang;
const std::string PISAAtom::PACKET_IDENTIFIER = "packet";
const std::string PISAAtom::STATE_SCALAR_IDENTIFIER  = "scalar";
const std::string PISAAtom::STATE_ARRAY_IDENTIFIER = "array";

PISAAtom::PISAAtom(const clang::Stmt * stmt,
                       const std::string & t_name,
                       const ScalarInitializer & scalar_initializer,
                       const ArrayInitializer  & array_initializer)
    : name_(t_name),
      function_body_(rewrite_into_pisa_ops(stmt)),
      function_definition_("[] (Packet  & " + PACKET_IDENTIFIER       + " __attribute__((unused)), " +
                           "StateScalar & " + STATE_SCALAR_IDENTIFIER + " __attribute__((unused)), " +
                           "StateArray  & " + STATE_ARRAY_IDENTIFIER  + " __attribute__((unused))) {\n" +
                           function_body_ + "\n }"),
      state_scalars_used_(gen_var_list(stmt, {{VariableType::STATE_ARRAY, false}, {VariableType::STATE_SCALAR, true},  {VariableType::PACKET, false}})),
      state_arrays_used_(gen_var_list(stmt, {{VariableType::STATE_ARRAY, true},  {VariableType::STATE_SCALAR, false}, {VariableType::PACKET, false}})),
      atom_definition_("Atom " + name_ + "(" + function_definition_ + ", "
                        + scalar_init_string(state_scalars_used_, scalar_initializer)
                        + ", " + array_init_string(state_arrays_used_, array_initializer) + ");")
{}

std::string PISAAtom::scalar_init_string(const VariableSet & state_scalars_used, const ScalarInitializer & scalar_initializer) const {
  // Generate initial values for all state scalars used within stmt
  std::string scalar_init_str = "StateScalar(std::map<std::string, int>";
  scalar_init_str += "{";
  for (const auto & scalar_var : state_scalars_used) {
    scalar_init_str += "{\"" + scalar_var + "\", " + std::to_string(scalar_initializer.at(scalar_var)) + "},";
  }
  scalar_init_str += '}'; // to close std::map constructor's initializer list
  scalar_init_str += ")"; // to close FieldContainer constructor's left parenthesis
  return scalar_init_str;
}

std::string PISAAtom::array_init_string(const VariableSet & state_arrays_used, const ArrayInitializer & array_initializer) const {
  // Generate initial values for all state arrays used within stmt
  std::string array_init_str = "StateArray(std::map<std::string, std::vector<int>>";
  array_init_str += "{";
  for (const auto & array_var : state_arrays_used) {
    array_init_str += "{\"" + array_var + "\", std::vector<int>("
                      + std::to_string(array_initializer.at(array_var).first) + ", " + std::to_string(array_initializer.at(array_var).second)
                      /* The above uses the second constructor from here: http://en.cppreference.com/w/cpp/container/vector/vector */
                      + ")},";
  }
  array_init_str += "}"; // to close std::map constructor's initializer list
  array_init_str += ")"; // to close FieldContainer constructor's left parenthesis
  return array_init_str;
}

std::string PISAAtom::rewrite_into_pisa_ops(const clang::Stmt * stmt) const {
  assert_exception(stmt);

  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += rewrite_into_pisa_ops(child) + ";";
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    std::string ret;
    ret += "if (" + rewrite_into_pisa_ops(if_stmt->getCond()) + ") {" + rewrite_into_pisa_ops(if_stmt->getThen()) + "; }";
    if (if_stmt->getElse() != nullptr) {
      ret += "else {" + rewrite_into_pisa_ops(if_stmt->getElse()) + "; }";
    }
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return rewrite_into_pisa_ops(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + rewrite_into_pisa_ops(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return     rewrite_into_pisa_ops(cond_op->getCond()) + " ? "
             + rewrite_into_pisa_ops(cond_op->getTrueExpr()) + " : "
             + rewrite_into_pisa_ops(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt)) {
    const auto * member_expr = dyn_cast<MemberExpr>(stmt);
    // All packet fields are of the type p(...) in banzai
    // N.B. the PISA code overloads the () operator.
    return   PACKET_IDENTIFIER + "(\"" + clang_value_decl_printer(member_expr->getMemberDecl()) + "\")";
  } else if (isa<DeclRefExpr>(stmt)) {
    // All state variables are of the type s(...) in banzai
    // N.B. Again by overloading the () operator
    const auto * decl_expr = dyn_cast<DeclRefExpr>(stmt);
    return   STATE_SCALAR_IDENTIFIER + "(\"" + clang_value_decl_printer(decl_expr->getDecl()) + "\")";
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    const auto * array_expr = dyn_cast<ArraySubscriptExpr>(stmt);
    return   STATE_ARRAY_IDENTIFIER + "(\"" + clang_stmt_printer(array_expr->getBase()) + "\")"
             + ".at(static_cast<uint64_t>(" + rewrite_into_pisa_ops(array_expr->getIdx()) + "))";
  } else if (isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + rewrite_into_pisa_ops(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + rewrite_into_pisa_ops(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return rewrite_into_pisa_ops(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = rewrite_into_pisa_ops(child);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else if (isa<NullStmt>(stmt)) {
    return "";
  } else {
    throw std::logic_error("rewrite_into_pisa_ops cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
