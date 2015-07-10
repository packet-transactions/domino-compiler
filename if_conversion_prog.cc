#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/ReplacementsYaml.h"
#include "llvm/Support/YAMLTraits.h"
#include "clang_utility_functions.h"
#include "if_conversion_handler.h"
#include "function_decl_handler.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory if_conversion(""
"This allows us to rewrite if statements into ternary operators"
" and recursively get rid of all branches, from the innermost to"
" the outermost ones.");

int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, if_conversion);
  RefactoringTool refactoring_tool(op.getCompilations(), op.getSourcePathList());

  // Set up AST matcher callbacks for if statements
  IfConversionHandler if_conversion_handler;
  MatchFinder find_function_decl;
  find_function_decl.addMatcher(functionDecl().bind("functionDecl"), & if_conversion_handler);
  refactoring_tool.run(newFrontendActionFactory(& find_function_decl).get());

  return 0;
}
