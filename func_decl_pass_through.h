#ifndef FUNC_DECL_PASS_THROUGH_H_
#define FUNC_DECL_PASS_THROUGH_H_

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class FuncDeclPassThrough : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;
  auto output() const { return output_; }
 private:
  /// Output from FuncDeclPassThrough pass
  std::string output_ = "";
};

#endif  // FUNC_DECL_PASS_THROUGH_H_
