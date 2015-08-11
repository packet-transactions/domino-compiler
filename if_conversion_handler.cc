#include "if_conversion_handler.h"

#include <iostream>

#include "pkt_func_transform.h"
#include "clang_utility_functions.h"
#include "identifier_census.h"

using namespace clang;
using std::placeholders::_1;
using std::placeholders::_2;

std::string IfConversionHandler::transform(const TranslationUnitDecl * tu_decl) {
  unique_identifiers_ = UniqueIdentifiers(identifier_census(tu_decl));
  return pkt_func_transform(tu_decl, std::bind(& IfConversionHandler::if_convert_body, this, _1, _2));
}

std::pair<std::string, std::vector<std::string>> IfConversionHandler::if_convert_body(const Stmt * function_body, const std::string & pkt_name) const {
  assert(function_body);

  std::string output_ = "";
  std::vector<std::string> new_decls_ = {};

  // 1 is the C representation for true
  if_convert(output_,  new_decls_, "1", function_body, pkt_name);
  return make_pair(output_, new_decls_);
}

void IfConversionHandler::if_convert(std::string & current_stream,
                                     std::vector<std::string> & current_decls,
                                     const std::string & predicate,
                                     const Stmt * stmt,
                                     const std::string & pkt_name) const {
  if (isa<CompoundStmt>(stmt)) {
    for (const auto & child : stmt->children()) {
      if_convert(current_stream, current_decls, predicate, child, pkt_name);
    }
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);

    if (if_stmt->getConditionVariableDeclStmt()) {
      throw std::logic_error("We don't yet handle declarations within the test portion of an if\n");
    }

    // Create temporary variable to hold the if condition
    const auto condition_type_name = if_stmt->getCond()->getType().getAsString();
    const auto cond_variable       = unique_identifiers_.get_unique_identifier();
    const auto cond_var_decl       = condition_type_name + " " + cond_variable + ";";

    // Add cond var decl to the packet structure, so that all decls accumulate there
    current_decls.emplace_back(cond_var_decl);

    // Add assignment to new packet temporary here,
    // predicating it with the current predicate
    const auto pkt_cond_variable = pkt_name + "." + cond_variable;
    current_stream += pkt_cond_variable + " = (" + predicate + " ? (" + clang_stmt_printer(if_stmt->getCond()) + ") :  "
                                        + pkt_cond_variable + ");";

    // Create predicates for if and else block
    auto pred_within_if_block = "(" + predicate + " && " + pkt_cond_variable + ")";
    auto pred_within_else_block = "(" + predicate + " && !" + pkt_cond_variable + ")";

    // If convert statements within getThen block to ternary operators.
    if_convert(current_stream, current_decls, pred_within_if_block, if_stmt->getThen(), pkt_name);

    // If there is a getElse block, handle it recursively again
    if (if_stmt->getElse() != nullptr) {
      if_convert(current_stream, current_decls, pred_within_else_block, if_stmt->getElse(), pkt_name);
    }
  } else if (isa<BinaryOperator>(stmt)) {
    current_stream += if_convert_atomic_stmt(dyn_cast<BinaryOperator>(stmt), predicate);
  } else if (isa<DeclStmt>(stmt)) {
    // Just append statement as is, but check that this only happens at the
    // top level i.e. when predicate = "1" or true
    assert(predicate == "1");
    current_stream += clang_stmt_printer(stmt);
    return;
  } else {
    assert(false);
  }
}

std::string IfConversionHandler::if_convert_atomic_stmt(const BinaryOperator * stmt,
                                                        const std::string & predicate) const {
  assert(stmt);
  assert(stmt->isAssignmentOp());
  assert(not stmt->isCompoundAssignmentOp());

  // Create predicated version of BinaryOperator
  const std::string lhs = clang_stmt_printer(dyn_cast<BinaryOperator>(stmt)->getLHS());
  const std::string rhs = "(" + predicate + " ? (" + clang_stmt_printer(stmt->getRHS()) + ") :  " + lhs + ")";
  return (lhs + " = " + rhs + ";");
}
