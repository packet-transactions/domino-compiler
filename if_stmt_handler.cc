#include <iostream>
#include "clang_utility_functions.h"
#include "if_stmt_handler.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

void IfStmtHandler::run(const MatchFinder::MatchResult & t_result) {
  const auto * if_stmt = t_result.Nodes.getNodeAs<clang::IfStmt>("ifStmt");
  assert(if_stmt != nullptr);
  std::cout << "Found if_stmt" << std::endl
            << clang_stmt_printer(if_stmt) << std::endl;
  std::cout << "Condition" << std::endl
            << clang_stmt_printer(if_stmt->getCond()) << std::endl;
  assert(if_stmt->getThen() != nullptr);
  std::cout << "Then" << std::endl
            << clang_stmt_printer(if_stmt->getThen()) << std::endl;
  std::cout << "Else" << std::endl
            << (if_stmt->getElse() != nullptr ? clang_stmt_printer(if_stmt->getElse()) : "empty") << std::endl;
  if (if_stmt->getConditionVariableDeclStmt()) {
    throw std::logic_error("We don't yet handle declarations within the test portion of an if\n");
  }

  // Create temporary variable to hold the if condition
  std::string tmp_var_decl = "";
  auto condition_type_name = if_stmt->getCond()->getType().getAsString();
  std::cout << "Type name for condition is " << condition_type_name << std::endl;
  std::cout << "Temp. var. declaration is " << condition_type_name + " tmp__" + std::to_string(var_counter_++) + " = " + clang_stmt_printer(if_stmt->getCond()) << ";" << std::endl;

  // Convert statements within then block to ternary operators.
  if (not isa<CompoundStmt>(if_stmt->getThen())) {
    // For now, error out if there's an if statement without braces (i.e. not CompoundStmt)
    throw std::logic_error("We don't yet handle if statments without braces\n");
  }

  // Print out children in CompoundStmt
  assert(isa<CompoundStmt>(if_stmt->getThen()));
  for (const auto & child : if_stmt->getThen()->children()) {
    // The "atomic" statements can only be binary operators,
    // declarations, or conditional operators
    assert(not isa<CompoundStmt>(child));
    assert(isa<DeclStmt>(child) or isa<BinaryOperator>(child) or isa<ConditionalOperator>(child));
    std::cout << "child: " << clang_stmt_printer(child) << std::endl;

    // Replace an atomic statement with a ternary version of itself
    replace_atomic_stmt(child);
  }
}
