#ifndef FUNC_TRANSFORM_HANDLER_C_
#define FUNC_TRANSFORM_HANDLER_C_

#include <iostream>
#include "clang_utility_functions.h"
#include "func_transform_handler.h"

using namespace clang;
using namespace clang::ast_matchers;

template <class TransformType>
void FuncTransformHandler<TransformType>::run(const MatchFinder::MatchResult & t_result) {
  const auto * function_decl = t_result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
  assert(function_decl != nullptr);

  // Get name of packet variable
  assert(function_decl->getNumParams() == 1);
  const auto * pkt_param = function_decl->getParamDecl(0);
  const auto pkt_type  = function_decl->getParamDecl(0)->getType().getAsString();
  const auto pkt_name = clang_value_decl_printer(pkt_param);

  assert(function_decl->getBody() != nullptr);
  auto ret = transformer_.transform(function_decl->getBody(), pkt_name);

  // Get output string and new decls
  out_str_ = "void func( " + pkt_type + " " +  pkt_name + ") { " + ret.first + "}\n";
  new_decls_ = ret.second;
}

#endif  // FUNC_TRANSFORM_HANDLER_C_
