#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/ReplacementsYaml.h"
#include "llvm/Support/YAMLTraits.h"
#include "clang_utility_functions.h"
#include "partitioning_handler.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory partitioning_program(""
"Partition program following if conversion");

int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, partitioning_program);
  RefactoringTool refactoring_tool(op.getCompilations(), op.getSourcePathList());

  // Set up AST matcher callbacks to do partitioning
  PartitioningHandler partitioning_handler;
  MatchFinder find_function_decl;
  find_function_decl.addMatcher(functionDecl().bind("functionDecl"), & partitioning_handler);
  refactoring_tool.run(newFrontendActionFactory(& find_function_decl).get());

  return 0;
}
