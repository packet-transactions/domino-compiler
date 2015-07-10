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
  IfConversionHandler if_conversion_handler(refactoring_tool.getReplacements());
  MatchFinder find_if_stmt;
  find_if_stmt.addMatcher(ifStmt().bind("ifStmt"), & if_conversion_handler);
  refactoring_tool.run(newFrontendActionFactory(& find_if_stmt).get());

  // Set up AST matcher callbacks for function declaration to add declarations
  FunctionDeclHandler function_decl_handler(refactoring_tool.getReplacements(), if_conversion_handler.get_decls());
  MatchFinder find_function_decl;
  find_function_decl.addMatcher(functionDecl().bind("functionDecl"), & function_decl_handler);
  refactoring_tool.run(newFrontendActionFactory(& find_function_decl).get());

  // Write into YAML object
  TranslationUnitReplacements replace_yaml;
  replace_yaml.MainSourceFile = argv[1];
  replace_yaml.Context = "no_context";
  for (const auto &r : refactoring_tool.getReplacements())
    replace_yaml.Replacements.push_back(r);

  // Serialize to stdout
  llvm::yaml::Output yaml_stream(llvm::outs());
  yaml_stream << replace_yaml;

  return 0;
}
