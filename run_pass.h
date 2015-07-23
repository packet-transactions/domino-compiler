#ifndef RUN_PASS_H_
#define RUN_PASS_H_

#include <string>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

template <class HandlerType, class ReturnType, typename ...Fargs>
ReturnType run_pass(clang::tooling::CommonOptionsParser & op, Fargs... args) {
  clang::tooling::RefactoringTool refactoring_tool(op.getCompilations(), op.getSourcePathList());

  // Set up matcher for translation unit declaration
  clang::ast_matchers::MatchFinder find_tu_decl;

  // Set up AST matcher callback
  HandlerType handler(args...);
  find_tu_decl.addMatcher(clang::ast_matchers::decl().bind("decl"), & handler);

  // Run tool
  refactoring_tool.run(clang::tooling::newFrontendActionFactory(& find_tu_decl).get());

  // Return output
  return handler.output();
}

#endif
