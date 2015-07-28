#ifndef RUN_PASS_H_
#define RUN_PASS_H_

#include <string>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

template <class HandlerType, class ReturnType, typename ...Fargs>
ReturnType run_pass(clang::tooling::CommonOptionsParser & op, const clang::ast_matchers::DeclarationMatcher & decl_matcher, Fargs... args) {
  clang::tooling::RefactoringTool refactoring_tool(op.getCompilations(), op.getSourcePathList());

  // Set up matcher
  clang::ast_matchers::MatchFinder find_ast_fragment;

  // Set up AST matcher callback
  HandlerType handler(args...);
  find_ast_fragment.addMatcher(decl_matcher, & handler);

  // Run tool
  refactoring_tool.run(clang::tooling::newFrontendActionFactory(& find_ast_fragment).get());

  // Return output
  return handler.output();
}

#endif // RUN_PASS_H_
