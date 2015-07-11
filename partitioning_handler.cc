#include <map>
#include <iostream>
#include "partitioning_handler.h"
#include "clang_utility_functions.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

void PartitioningHandler::run(const MatchFinder::MatchResult & t_result) {
  const FunctionDecl *function_decl_expr = t_result.Nodes.getNodeAs<clang::FunctionDecl>("functionDecl");
  assert(function_decl_expr != nullptr);
  assert(isa<CompoundStmt>(function_decl_expr->getBody()));
  InstructionVector useful_ops;
  for (const auto & child : function_decl_expr->getBody()->children()) {
    assert(child);
    assert(isa<DeclStmt>(child) or isa<BinaryOperator>(child));
    if (isa<BinaryOperator>(child)) {
      const auto * bin_op = dyn_cast<BinaryOperator>(child);
      assert(bin_op->isAssignmentOp());
      assert(isa<DeclRefExpr>(bin_op->getLHS()));
      useful_ops.push_back(bin_op);
    }
  }

  // Partition into a pipeline
  auto partitioning = partition_into_pipeline(useful_ops);

  // Check for pipeline-wide variables
  check_for_pipeline_vars(partitioning);

  // Print out partitioning
  for (uint32_t i = 0; i < partitioning.size(); i++) {
    std::cout << "Clock " << i << " : " << std::endl;
    for (const auto & op : partitioning.at(i)) {
      std::cout << " { " << clang_stmt_printer(op) << " } " << " ";
    }
    std::cout << std::endl;
  }
}

bool PartitioningHandler::op_reads_var(const BinaryOperator * op, const DeclRefExpr * var) const {
  // This is an ugly hack using string search (to say the least!)
  // TODO: Fix this at some later point in time once it is clear how to do it.
  return (clang_stmt_printer(op).find(clang_stmt_printer(var)) != std::string::npos);
}

bool PartitioningHandler::depends(const BinaryOperator * op1, const BinaryOperator * op2) const {
  // We are being a little conservative here and flagging all 
  // three "textbook" dependencies:
  // Read After Write,
  // Write After Write,
  // and Write After Read.
  // Although renaming can prevent the latter two.

  // assume and check that op1 precedes op2 in program order
  assert(op1->getLocStart() < op2->getLocStart());
  assert(isa<DeclRefExpr>(op1->getLHS()));
  assert(isa<DeclRefExpr>(op2->getLHS()));

  // op1 writes a variable (LHS) that op2 reads. (Read After Write)
  if (op_reads_var(op2, dyn_cast<DeclRefExpr>(op1->getLHS()))) {
    return true;
  }

  // op1 writes the same variable that op2 writes (Write After Write)
  if (clang_stmt_printer(op1->getLHS()) == clang_stmt_printer(op2->getLHS())) {
    return true;
  }

  // op1 reads a variable that op2 writes (Write After Read)
  if (op_reads_var(op1, dyn_cast<DeclRefExpr>(op2->getLHS()))) {
    return true;
  }

  return false;
}

PartitioningHandler::InstructionPartitioning PartitioningHandler::partition_into_pipeline(const InstructionVector & inst_vector) const {
  // Create dag of instruction dependencies.
  std::map<const BinaryOperator *, InstructionVector> succ_graph;
  std::map<const BinaryOperator *, InstructionVector> pred_graph;
  for (const auto & op : inst_vector) {
    succ_graph[op] = {};
    pred_graph[op] = {};
  }

  for (uint32_t i = 0; i < inst_vector.size(); i++) {
    for (uint32_t j = i + 1; j < inst_vector.size(); j++) {
      if (depends(inst_vector.at(i), inst_vector.at(j))) {
        // edge from i ---> j
        succ_graph.at(inst_vector.at(i)).emplace_back(inst_vector.at(j));
        pred_graph.at(inst_vector.at(j)).emplace_back(inst_vector.at(i));
      }
    }
  }

  // Keep track of what needs to be partitioned
  InstructionVector to_partition = inst_vector;

  // The partitioning itself
  InstructionPartitioning partitioning;

  while (not to_partition.empty()) {
    // Find elements with no predecessors
    std::vector<const BinaryOperator *> chosen_elements;
    for (const auto & candidate : to_partition) {
      if (pred_graph.at(candidate).empty()) {
        chosen_elements.emplace_back(candidate);
      }
    }
    assert(not chosen_elements.empty());

    // append to partitioning
    partitioning.emplace_back(chosen_elements);

    // remove chosen_elements from graph by deleting each element
    for (const auto & chosen_element : chosen_elements) {
      // Delete each element from all its successors' pred_graph
      // Delete each element from all its predessors's succ_graph (trivially true, because it has no predessors)
      for (const auto & successor : succ_graph.at(chosen_element)) {
        // delete chosen_element from pred_graph.at(successor)
        pred_graph.at(successor).erase(std::remove(pred_graph.at(successor).begin(), pred_graph.at(successor).end(), chosen_element));
      }

      // Remove chosen_element from pred_graph and succ_graph
      succ_graph.erase(chosen_element);
      pred_graph.erase(chosen_element);
    }

    // remove chosen_elements from to_partition
    for (const auto & chosen_element : chosen_elements) {
      to_partition.erase(std::remove(to_partition.begin(), to_partition.end(), chosen_element));
    }
  }

  return partitioning;
}

std::vector<std::string> PartitioningHandler::check_for_pipeline_vars(const InstructionPartitioning & partitioning) const {
  for (const auto & inst_vector : partitioning) {
    for (const auto & inst : inst_vector) {
      assert(isa<BinaryOperator>(inst));
      assert(isa<DeclRefExpr>(inst->getLHS()));
      // TODO: Fix this. Another string-typing hack: all local vars are assumed to have "__" within them.
      if (clang_stmt_printer(dyn_cast<DeclRefExpr>(inst->getLHS())).find("__") != std::string::npos) {
        std::cerr << "check_for_pipeline_vars, ignoring local variable: " << clang_stmt_printer(dyn_cast<DeclRefExpr>(inst->getLHS())) << "\n";
      } else {
        // Do something non-trivial here
      }
    }
  }
  return {};
}
