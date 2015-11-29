#ifndef DESUGAR_COMPOUND_ASSIGNMENT_H_
#define DESUGAR_COMPOUND_ASSIGNMENT_H_

#include "ast_visitor.h"

class DesugarCompAssignment : public AstVisitor {
 protected:
  /// Desugar compound assignment, by replacing:
  /// E1 op= E2 with E1 = E1 op E2
  /// This is almost correct (quoting from cppreference.com)
  /// "except that the expression E1 is evaluated only once
  ///and that it behaves as a single operation with respect
  ///to indeterminately-sequenced function calls (e.g. in f(a+= b, g()),
  /// the += is either not started at all or is
  ///completed as seen from inside g())."
  /// I don't think that's reasonable code anyway, so for
  /// now I am ignoring it (TODO).
  std::string ast_visit_bin_op(const clang::BinaryOperator * bin_op) override;

 private:
  /// get_underlying_op from a compound assignment operator
  clang::BinaryOperator::Opcode get_underlying_op(const clang::BinaryOperator::Opcode & comp_asgn_op) const;
};

#endif // DESUGAR_COMPOUND_ASSIGNMENT_H_
