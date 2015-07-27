#include <map>
#include <iostream>
#include "partitioning_handler.h"
#include "clang_utility_functions.h"
#include "expr_functions.h"

using namespace clang;
using namespace clang::ast_matchers;

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
      assert(check_expr_type(bin_op->getLHS()));
      useful_ops.push_back(bin_op);
    }
  }

  // Partition into a pipeline
  const auto partitioning = partition_into_pipeline(useful_ops);

  // Check for pipeline-wide variables
  check_for_pipeline_vars(partitioning);

  // Print out partitioning
  for (const auto & pair : partitioning) {
    std::cout << " { " << clang_stmt_printer(pair.first) << " } " << " " << pair.second << std::endl;
  }
}

bool PartitioningHandler::op_reads_var(const BinaryOperator * op, const Expr * var) const {
  // This is an ugly hack using string search (to say the least!)
  // TODO: Fix this at some later point in time once it is clear how to do it.
  assert(op);
  assert(var);
  assert(check_expr_type(var));
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
  assert(check_expr_type(op1->getLHS()));
  assert(check_expr_type(op2->getLHS()));

  // op1 writes a variable (LHS) that op2 reads. (Read After Write)
  if (op_reads_var(op2, op1->getLHS())) {
    return true;
  }

  // op1 writes the same variable that op2 writes (Write After Write)
  if (clang_stmt_printer(op1->getLHS()) == clang_stmt_printer(op2->getLHS())) {
    return true;
  }

  // op1 reads a variable that op2 writes (Write After Read)
  if (op_reads_var(op1, op2->getLHS())) {
    return true;
  }

  return false;
}

PartitioningHandler::InstructionPartitioning PartitioningHandler::partition_into_pipeline(const InstructionVector & inst_vector) const {
  // Create dag of instruction dependencies.
  std::map<const BinaryOperator *, InstructionVector> pred_graph;
  std::map<std::pair<const BinaryOperator *, const BinaryOperator *>, uint32_t> edge_graph;
  for (const auto & op : inst_vector) {
    pred_graph[op] = {};
  }

  for (uint32_t i = 0; i < inst_vector.size(); i++) {
    for (uint32_t j = i + 1; j < inst_vector.size(); j++) {
      if (depends(inst_vector.at(i), inst_vector.at(j))) {
        // edge from i ---> j
        pred_graph.at(inst_vector.at(j)).emplace_back(inst_vector.at(i));
        edge_graph[std::make_pair(inst_vector.at(i), inst_vector.at(j))] = 1;
      }
    }
  }

  // Keep track of what needs to be partitioned
  InstructionVector to_partition = inst_vector;

  // The partitioning itself, map from const BinaryOperator * to timestamp
  InstructionPartitioning partitioning;

  while (not to_partition.empty()) {
    // Find next instruction
    const BinaryOperator * next_inst = nullptr;
    for (const auto & candidate : to_partition) {
      if (std::accumulate(pred_graph.at(candidate).begin(), pred_graph.at(candidate).end(),
                          true,
                          [&to_partition] (const auto & acc, const auto & x)
                          { return acc and (std::find(to_partition.begin(), to_partition.end(), x) == to_partition.end());})) {
        next_inst = candidate;
        break;
      }
    }
    assert(next_inst);

    // Find time for next_inst
    const uint32_t next_inst_time = std::accumulate(pred_graph.at(next_inst).begin(),
                                                    pred_graph.at(next_inst).end(),
                                                    static_cast<uint32_t>(0),
                                                    [&partitioning, &edge_graph, &next_inst] (const auto & acc, const auto & x)
                                                    { return std::max(acc, partitioning.at(x) + edge_graph.at(std::make_pair(x, next_inst)));});

    // append to partitioning, remove from to_partition
    partitioning[next_inst] = next_inst_time;
    to_partition.erase(std::remove(to_partition.begin(), to_partition.end(), next_inst));
  }

  return partitioning;
}

bool PartitioningHandler::check_for_pipeline_vars(const InstructionPartitioning & partitioning) const {
  // State variables occurences.
  // This is a map from the name of the state variable
  // to a set listing the stage ids where this state variable is read/written.
  std::map<std::string, std::set<int>> state_var_reads;
  std::map<std::string, std::set<int>> state_var_writes;

  for (const auto & pair : partitioning) {
    const auto inst = pair.first;
    assert(isa<BinaryOperator>(inst));
    for (const auto & write_var : ExprFunctions::get_all_state_vars(inst->getLHS())) {
      if (state_var_writes.find(write_var) == state_var_writes.end()) {
        state_var_writes[write_var] = std::set<int>();
      }
      state_var_writes.at(write_var).emplace(pair.second);
    }
    for (const auto & read_var : ExprFunctions::get_all_state_vars(inst->getRHS())) {
      if (state_var_reads.find(read_var) == state_var_reads.end()) {
        state_var_reads[read_var] = std::set<int>();
      }
      state_var_reads.at(read_var).emplace(pair.second);
    }
  }

  // Now, go through all state variables that are actually written.
  // If the state var is never written, there is nothing to worry about,
  // though it isn't clear why something like that would ever be useful.
  for (const auto & pair : state_var_writes) {
    const auto var_name = pair.first;
    std::cerr << "Checking variable name " << var_name << " for pipeline-wide sharing" << std::endl;

    // Find all reads for var_name, if they exist at all.
    // Again, not sure why there would be a variable
    // with no reads and only writes.
    if (state_var_reads.find(var_name) != state_var_reads.end()) {
      // Get sets of all read and write stage ids
      const auto read_stages = state_var_reads.at(var_name);
      const auto write_stages = state_var_writes.at(var_name);

      // Check if var_name is ever defined AFTER being used.
      // i.e. if there exists a pair (read_stage_id, write_stage_id),
      // where read_stage_id < write_stage_id, and such that var_name
      // is read in read_stage_id and written in write_stage_id
      // TODO: Is it < 'or' <=
      for (const auto & read_stage_id : read_stages) {
        // Find first write stage id iterator such that it is strictly
        // greater than read_stage_id (http://en.cppreference.com/w/cpp/container/set/upper_bound)
        auto first_write_after_read = write_stages.upper_bound(read_stage_id);
        if (first_write_after_read != write_stages.end()) {
          // pipeline-wide shared variable
          std::cout << var_name << " needs pipeline-wide sharing"
                    << ", read in " << read_stage_id
                    << ", written in " << *first_write_after_read
                    << std::endl;
          return true;
        }
      }
    }
  }
  return false;
}
