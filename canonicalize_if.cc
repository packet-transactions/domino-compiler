#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/ReplacementsYaml.h"
#include "llvm/Support/YAMLTraits.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory canonicalize_if_stmts(""
"Replace if statements with canonical variants: each if statement contains"
"exactly one simple statement, arithmetic or boolean. This allows us to"
"rewrite if statements into ternary operators and recursively get rid of all"
"branches, from the innermost to the outermost ones.");

class IfStmtHandler : public MatchFinder::MatchCallback {
 public:
  /// Constructor: Pass replacements member of a RefactoringTool as an argument
  IfStmtHandler(Replacements & t_replace) : replace_(t_replace) {}

  /// Callback whenever there's a match
  virtual void run(const MatchFinder::MatchResult & t_result) override {
    const auto * if_stmt = t_result.Nodes.getNodeAs<clang::IfStmt>("ifStmt");
    assert(if_stmt != nullptr);
  }

 private:
  Replacements & replace_;
};

int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, canonicalize_if_stmts);
  RefactoringTool refactoring_tool(op.getCompilations(), op.getSourcePathList());

  // Set up AST matcher callbacks for if statements
  IfStmtHandler if_stmt_handler(refactoring_tool.getReplacements());
  MatchFinder find_if_stmt;
  find_if_stmt.addMatcher(ifStmt().bind("ifStmt"), & if_stmt_handler);
  refactoring_tool.run(newFrontendActionFactory(& find_if_stmt).get());

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
