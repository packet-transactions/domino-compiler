#ifndef SINGLE_PASS_H_
#define SINGLE_PASS_H_

#include <functional>

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

/// Single pass over a translation unit.
/// By default, just parse the translation unit, and print it out as such.
/// If you are interested in rewriting specific parts of it, specify those alone.
template <class OutputType>
class SinglePass {
 public:
  /// Run a single pass over a given CommonOptionsParser object
  /// Using the given transformer object
  SinglePass(clang::tooling::CommonOptionsParser & op,
             const std::function<OutputType(const clang::TranslationUnitDecl *)> & t_transformer);

  /// Output from SinglePass
  auto output() const { return output_; }
 private:
  /// Refactoring tool
  clang::tooling::RefactoringTool refactoring_tool_;

  /// Inherit from MatchCallback to call transformer
  class TransformHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
   public:
    TransformHandler(const std::function<OutputType(const clang::TranslationUnitDecl *)> & t_transformer) : transformer_(t_transformer) {}
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;
    auto output() const { return output_; }
   private:
    OutputType output_ = {};
    /// transformer function
    std::function<OutputType(const clang::TranslationUnitDecl *)> transformer_;
  };
  TransformHandler transform_handler_;

  /// Find appropriate AST fragment, in our case the entire Translation Unit
  clang::ast_matchers::MatchFinder find_ast_fragment_ = {};

  /// Output from parsing
  OutputType output_ = {};
};

template <class OutputType>
SinglePass<OutputType>::SinglePass(clang::tooling::CommonOptionsParser & op,
                       const std::function<OutputType(const clang::TranslationUnitDecl *)> & t_transformer)
    : refactoring_tool_(op.getCompilations(), op.getSourcePathList()),
      transform_handler_(t_transformer) {
  find_ast_fragment_.addMatcher(clang::ast_matchers::decl().bind("decl"), & transform_handler_);
  refactoring_tool_.run(clang::tooling::newFrontendActionFactory(& find_ast_fragment_).get());
  output_ = transform_handler_.output();
}

template <class OutputType>
void SinglePass<OutputType>::TransformHandler::run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) {
  const auto * decl = t_result.Nodes.getNodeAs<clang::Decl>("decl");
  assert(decl != nullptr);

  if (not llvm::isa<clang::TranslationUnitDecl>(decl)) return;

  // Handle only translation unit decls
  assert(llvm::isa<clang::TranslationUnitDecl>(decl));

  // Carry out transformation
  output_ = transformer_(llvm::dyn_cast<clang::TranslationUnitDecl>(decl));
}

#endif  // SINGLE_PASS_H_
