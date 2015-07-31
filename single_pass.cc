#include "single_pass.h"
#include "clang_utility_functions.h"

using namespace clang;
using namespace clang::ast_matchers;

SinglePass::SinglePass(clang::tooling::CommonOptionsParser & op,
                       const std::function<std::string(const clang::TranslationUnitDecl *)> & t_transformer)
    : refactoring_tool_(op.getCompilations(), op.getSourcePathList()),
      transform_handler_(t_transformer) {
  find_ast_fragment_.addMatcher(decl().bind("decl"), & transform_handler_);
  refactoring_tool_.run(clang::tooling::newFrontendActionFactory(& find_ast_fragment_).get());
  output_ = transform_handler_.output();
}

void SinglePass::TransformHandler::run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) {
  const auto * decl = t_result.Nodes.getNodeAs<Decl>("decl");
  assert(decl != nullptr);

  if (not isa<TranslationUnitDecl>(decl)) return;

  // Handle only translation unit decls
  assert(isa<TranslationUnitDecl>(decl));

  // Carry out transformation
  output_ = transformer_(dyn_cast<TranslationUnitDecl>(decl));
}
