#ifndef IF_STMT_HANDLER_H_
#define IF_STMT_HANDLER_H_

#include <random>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"

class IfStmtHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  /// Constructor: Pass replacements member of a RefactoringTool as an argument
  IfStmtHandler(clang::tooling::Replacements & t_replace) : replace_(t_replace) {}

  /// Callback whenever there's a match
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;

  /// Retun all declarations found so far
  auto get_decls() const { return decl_strings_; }

 private:
  /// Process one half of a branch i.e. either if or else branch
  void process_if_branch(const clang::CompoundStmt * stmt, clang::SourceManager & source_manager, const std::string & cond_variable);

  /// Replace an atomic (clang::BinaryOperator) statement
  /// with a conditional version of it
  void replace_atomic_stmt(const clang::BinaryOperator * stmt, clang::SourceManager & source_manager, const std::string & cond_variable);

  /// Remove token at specific SourceLocation
  void remove_token(const clang::SourceLocation & loc, const clang::ast_matchers::MatchFinder::MatchResult & t_result);

  /// Get CharSourceRange for a given SourceLocation,
  /// Using GetBeginningOfToken and getLocForEndOfToken
  clang::CharSourceRange get_src_range_for_loc(const clang::SourceLocation & loc, const clang::ast_matchers::MatchFinder::MatchResult & t_result) const;

  /// Check whether if_stmt has any nested ifs inside
  bool check_for_nesting(const clang::IfStmt * if_stmt) const;

  /// Set of all new variable declarations that were created
  std::set<std::string> decl_strings_ = {};

  /// Reference to replacements object
  clang::tooling::Replacements & replace_;

  /// Random engine to generate random number
  std::default_random_engine prng_ = std::default_random_engine(std::random_device()());

  /// Uniform distribution to generate random variable numbers
  std::uniform_int_distribution<int> uniform_dist_ = std::uniform_int_distribution<int>(1, 10000);
};

#endif  // IF_STMT_HANDLER_H_
