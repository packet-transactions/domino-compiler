#ifndef STATE_VAR_DECL_HANDLER_H_
#define STATE_VAR_DECL_HANDLER_H_

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class StateVarDeclHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;
  auto output() const { return output_; }
 private:
  std::string output_ = "";
};

#endif  // STATE_VAR_DECL_HANDLER_H_
