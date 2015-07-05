#ifndef IF_STMT_HANDLER_H_
#define IF_STMT_HANDLER_H_

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

 private:
  void replace_atomic_stmt(const clang::Stmt * stmt, clang::SourceManager & source_manager, const std::string & cond_variable);

  clang::tooling::Replacements & replace_;
  uint8_t var_counter_ = 0;
};

#endif  // IF_STMT_HANDLER_H_
