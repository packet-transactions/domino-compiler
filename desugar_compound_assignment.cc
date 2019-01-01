#include "desugar_compound_assignment.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

using namespace clang;

BinaryOperator::Opcode DesugarCompAssignment::get_underlying_op(const BinaryOperator::Opcode & comp_asgn_op) const {
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

std::string DesugarCompAssignment::ast_visit_bin_op(const BinaryOperator * bin_op) {
  assert_exception(bin_op);
  std::string ret;
  if (isa<CompoundAssignOperator>(bin_op)) {
    // Handle compound assignments alone
    return ast_visit_stmt(bin_op->getLHS()) + "=" + ast_visit_stmt(bin_op->getLHS())
           + std::string(BinaryOperator::getOpcodeStr(get_underlying_op(bin_op->getOpcode())))
           + ast_visit_stmt(bin_op->getRHS());
  } else {
    // Delegate to default base class method
    return AstVisitor::ast_visit_bin_op(bin_op);
  }
}
