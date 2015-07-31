#ifndef SINGLE_PASS_H_
#define SINGLE_PASS_H_

#include <string>
#include <functional>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

/// Single pass over a translation unit.
/// By default, just parse the translation unit, and print it out as such.
/// If you are interested in rewriting specific parts of it, specify those alone.
class SinglePass {
 public:
  /// Run a single pass over a given CommonOptionsParser object
  /// Using the given transformer object
  SinglePass(clang::tooling::CommonOptionsParser & op,
             const std::function<std::string(const clang::TranslationUnitDecl *)> & t_transformer);

  /// Output from SinglePass
  auto output() const { return output_; }
 private:
  /// Refactoring tool
  clang::tooling::RefactoringTool refactoring_tool_;

  /// Inherit from MatchCallback to call transformer
  class TransformHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
   public:
    TransformHandler(const std::function<std::string(const clang::TranslationUnitDecl *)> & t_transformer) : transformer_(t_transformer) {}
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;
    auto output() const { return output_; }
   private:
    std::string output_ = "";
    /// transformer function
    std::function<std::string(const clang::TranslationUnitDecl *)> transformer_;
  };
  TransformHandler transform_handler_;

  /// Find appropriate AST fragment, in our case the entire Translation Unit
  clang::ast_matchers::MatchFinder find_ast_fragment_ = {};

  /// String output from parsing
  std::string output_ = "";
};

#endif  // SINGLE_PASS_H_
