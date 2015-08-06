#include <iostream>
#include "clang/AST/AST.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang_utility_functions.h"
#include "pkt_func_transform.h"
#include "single_pass.h"

using namespace clang;
using namespace clang::tooling;

// Print out dependency graph
static std::pair<std::string, std::vector<std::string>> dep_graph_transform(const CompoundStmt * function_body, const std::string & pkt_name __attribute__ ((unused))) {
  // Newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Verify that it's in SSA
  std::set<std::string> assigned_vars;
  for (const auto * child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const auto pair = assigned_vars.emplace(clang_stmt_printer(lhs));
    if (pair.second == false) {
      throw std::logic_error("Program not in SSA form\n");
    }
  }

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
