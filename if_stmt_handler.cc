#include <iostream>
#include "clang_utility_functions.h"
#include "if_stmt_handler.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

void IfStmtHandler::run(const MatchFinder::MatchResult & t_result) {
  const auto * if_stmt = t_result.Nodes.getNodeAs<IfStmt>("ifStmt");
  assert(if_stmt != nullptr);
  assert(if_stmt->getThen() != nullptr);
  if (if_stmt->getConditionVariableDeclStmt()) {
    throw std::logic_error("We don't yet handle declarations within the test portion of an if\n");
  }

  // Create temporary variable to hold the if condition
  auto condition_type_name = if_stmt->getCond()->getType().getAsString();
  std::string tmp_var_decl = condition_type_name + " tmp__" + std::to_string(var_counter_++) + " = " + clang_stmt_printer(if_stmt->getCond()) + ";\n";

  // Convert statements within then block to ternary operators.
  if (not isa<CompoundStmt>(if_stmt->getThen())) {
    // For now, error out if there's an if statement without braces (i.e. not CompoundStmt)
    throw std::logic_error("We don't yet handle if statments without braces\n");
  }

  assert(isa<CompoundStmt>(if_stmt->getThen()));
  for (const auto & child : if_stmt->getThen()->children()) {
    // When we canonicalize a branch, we assume everything inside is already
    // canonicalized and isn't an IfStmt or a CompoundStmt on its own.
    assert(not isa<CompoundStmt>(child));
    if (isa<DeclStmt>(child)) {
      throw std::logic_error("We don't yet handle variable declarations within if statements\n");
    }

    // The "atomic" statements can only be binary operators
    // Everything at this level is represented as a BinaryOperator in clang,
    // Even something like a = x ? 5 : 4; is a BinaryOperator with two operands
    // a and (x ? 5 : 4).
    assert(isa<BinaryOperator>(child));

    // Replace an atomic statement with a ternary version of itself
    replace_atomic_stmt(child, *t_result.SourceManager);
  }
}

void IfStmtHandler::replace_atomic_stmt(const Stmt * stmt, SourceManager & source_manager) {
  assert(isa<BinaryOperator>(stmt));
  assert(dyn_cast<BinaryOperator>(stmt)->isAssignmentOp());
  assert(not dyn_cast<BinaryOperator>(stmt)->isCompoundAssignmentOp());

  // Create predicated version of BinaryOperator
  const std::string lhs = clang_stmt_printer(dyn_cast<BinaryOperator>(stmt)->getLHS());
  const std::string rhs = "(1 ? (" + clang_stmt_printer(dyn_cast<BinaryOperator>(stmt)->getRHS()) + ") :  (-1))" ;
  replace_.insert(Replacement(source_manager, stmt, lhs + " = " + rhs));
}
