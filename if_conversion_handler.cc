#include <iostream>
#include "clang_utility_functions.h"
#include "if_conversion_handler.h"

using namespace clang;
using namespace clang::ast_matchers;

void IfConversionHandler::run(const MatchFinder::MatchResult & t_result) {
  const auto * decl = t_result.Nodes.getNodeAs<Decl>("decl");
  assert(decl != nullptr);

  // Handle only translation unit decls
  if (isa<TranslationUnitDecl>(decl)) {
    // Get all decls by dyn casting decl into a DeclContext
    for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
      assert(child_decl);
      if (isa<FunctionDecl>(child_decl)) {
        const auto * function_decl = dyn_cast<FunctionDecl>(child_decl);

        // Get name of packet variable
        assert(function_decl->getNumParams() == 1);
        const auto * pkt_param = function_decl->getParamDecl(0);
        const auto pkt_name = clang_value_decl_printer(pkt_param);

        assert(function_decl->getBody() != nullptr);
        // 1 is the C representation for true
        if_convert(output_,  new_decls_, "1", function_decl->getBody(), pkt_name);

        // Rewrite output_
        output_ = "void func(Packet " + pkt_name + ") { " + output_ + "}\n";
      }
    }
  }
}

void IfConversionHandler::if_convert(std::string & current_stream,
                                     std::vector<std::string> & current_decls,
                                     const std::string & predicate,
                                     const Stmt * stmt,
                                     const std::string & pkt_name) const {
  // For unique renaming
  static uint8_t var_counter = 0;

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
    const auto cond_variable       = "tmp" + std::to_string(var_counter++);
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
