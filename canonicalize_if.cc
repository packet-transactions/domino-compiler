#include <iostream>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/ReplacementsYaml.h"
#include "llvm/Support/YAMLTraits.h"
#include "clang_utility_functions.h"

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
    std::cout << "Found if_stmt" << std::endl
              << clang_stmt_printer(if_stmt) << std::endl;
    std::cout << "Condition" << std::endl
              << clang_stmt_printer(if_stmt->getCond()) << std::endl;
    assert(if_stmt->getThen() != nullptr);
    std::cout << "Then" << std::endl
              << clang_stmt_printer(if_stmt->getThen()) << std::endl;
    std::cout << "Else" << std::endl
              << (if_stmt->getElse() != nullptr ? clang_stmt_printer(if_stmt->getElse()) : "empty") << std::endl;
    if (if_stmt->getConditionVariableDeclStmt()) {
      throw std::logic_error("We don't yet handle declarations within the test portion of an if\n");
    }

    // Create temporary variable to hold the if condition
    std::string tmp_var_decl = "";
    auto condition_type_name = if_stmt->getCond()->getType().getAsString();
    std::cout << "Type name for condition is " << condition_type_name << std::endl;
    std::cout << "Temp. var. declaration is " << condition_type_name + " tmp__" + std::to_string(var_counter_++) + " = " + clang_stmt_printer(if_stmt->getCond()) << ";" << std::endl;

    // Convert statements within then block to ternary operators.
    if (not isa<CompoundStmt>(if_stmt->getThen())) {
      // For now, error out if there's an if statement without braces (i.e. not CompoundStmt)
      throw std::logic_error("We don't yet handle if statments without braces\n");
    }

    // Print out children in CompoundStmt
    assert(isa<CompoundStmt>(if_stmt->getThen()));
    for (const auto & child : if_stmt->getThen()->children()) {
      // The "atomic" statements can only be binary operators,
      // declarations, or conditional operators
      assert(not isa<CompoundStmt>(child));
      assert(isa<DeclStmt>(child) or isa<BinaryOperator>(child) or isa<ConditionalOperator>(child));
      std::cout << "child: " << clang_stmt_printer(child) << std::endl;
    }
  }

 private:
  Replacements & replace_;
  uint8_t var_counter_ = 0;
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
