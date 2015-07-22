#include <iostream>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "state_var_decl_handler.h"
#include "if_conversion_handler.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory if_conversion(""
"This allows us to rewrite if statements into ternary operators"
" and recursively get rid of all branches, from the innermost to"
" the outermost ones.");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, if_conversion);
  RefactoringTool refactoring_tool(op.getCompilations(), op.getSourcePathList());

  // Set up matcher for translation unit declaration
  MatchFinder find_tu_decl;

  // Set up AST matcher callback for state variable declarations
  StateVarDeclHandler state_var_decl_handler;
  find_tu_decl.addMatcher(decl().bind("decl"), & state_var_decl_handler);

  // Set up AST matcher callbacks for if statements
  IfConversionHandler if_conversion_handler;
  find_tu_decl.addMatcher(decl().bind("decl"), & if_conversion_handler);

  // Run tool
  refactoring_tool.run(newFrontendActionFactory(& find_tu_decl).get());

  // Print out outputs
  std::cout << state_var_decl_handler.output() << "\n";
  std::cout << if_conversion_handler.output() << "\n";

  return 0;
}
