#include <iostream>
#include "clang/AST/AST.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "graph.h"
#include "clang_utility_functions.h"
#include "pkt_func_transform.h"
#include "single_pass.h"

using namespace clang;
using namespace clang::tooling;

/// Identify statements that read from and write to state
/// And create a back edge from the write back to the read.
/// This is really the crux of the compiler:
/// back edges from stateful writes to the next stateful read.
static Graph<const BinaryOperator *> handle_state_vars(const std::vector<const BinaryOperator *> & stmt_vector, const Graph<const BinaryOperator*> & dep_graph) {
  Graph<const BinaryOperator*> ret = dep_graph;
  std::map<std::string, const BinaryOperator *> state_reads;
  std::map<std::string, const BinaryOperator *> state_writes;
  for (const auto * stmt : stmt_vector) {
    const auto * lhs = stmt->getLHS()->IgnoreParenImpCasts();
    const auto * rhs = stmt->getRHS()->IgnoreParenImpCasts();
    if (isa<DeclRefExpr>(rhs)) {
      state_reads[clang_stmt_printer(rhs)] = stmt;
    } else if (isa<DeclRefExpr>(lhs)) {
      state_writes[clang_stmt_printer(lhs)] = stmt;
      const auto state_var = clang_stmt_printer(lhs);
      ret.add_edge(state_reads.at(state_var), state_writes.at(state_var));
      ret.add_edge(state_writes.at(state_var), state_reads.at(state_var));
    }
  }
  return ret;
}

}

/// Print out dependency graph
static std::pair<std::string, std::vector<std::string>> dep_graph_transform(const CompoundStmt * function_body, const std::string & pkt_name __attribute__ ((unused))) {
  // Newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Verify that it's in SSA
  // and append to a vector of const BinaryOperator *
  // in order of statement occurence.
  std::set<std::string> assigned_vars;
  std::vector<const BinaryOperator *> stmt_vector;
  for (const auto * child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const auto pair = assigned_vars.emplace(clang_stmt_printer(lhs));
    if (pair.second == false) {
      throw std::logic_error("Program not in SSA form\n");
    }
    stmt_vector.emplace_back(bin_op);
  }

  // Dependency graph creation
  Graph<const BinaryOperator *> dep_graph(clang_stmt_printer);
  for (const auto * stmt : stmt_vector) {
    dep_graph.add_node(stmt);
  }

  // Handle state variables specially
  dep_graph = handle_state_vars(stmt_vector, dep_graph);

    }
  }
  std::cerr << dep_graph.dot_output() << std::endl;

  return std::make_pair(clang_stmt_printer(function_body), new_decls);
}

static llvm::cl::OptionCategory dep_graph(""
"Print out dependency graph of the program as a dot file");

int main(int argc, const char ** argv) {
  // Parser options
  CommonOptionsParser op(argc, argv, dep_graph);

  // Parse file once and output dot file
  std::cout << SinglePass<std::string>(op, std::bind(pkt_func_transform, std::placeholders::_1, dep_graph_transform)).output();
}
