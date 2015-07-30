#ifndef FUNC_TRANSFORM_HANDLER_CC_
#define FUNC_TRANSFORM_HANDLER_CC_

#include <iostream>
#include "clang_utility_functions.h"
#include "func_transform_handler.h"

using namespace clang;
using namespace clang::ast_matchers;

template <class TransformType>
void FuncTransformHandler<TransformType>::run(const MatchFinder::MatchResult & t_result) {
  const auto * function_decl = t_result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
  assert(function_decl != nullptr);

  if (not is_packet_func(function_decl))
    return;

  // TODO: What happens when there is no packet variable available?
  assert(is_packet_func(function_decl));
  const auto * pkt_param = function_decl->getParamDecl(0);
  const auto pkt_type  = function_decl->getParamDecl(0)->getType().getAsString();
  const auto pkt_name = clang_value_decl_printer(pkt_param);

  assert(function_decl->getBody() != nullptr);
  auto ret = transformer_.transform(function_decl->getBody(), pkt_name);

  // Get output string and new decls
  out_str_ = "void func( " + pkt_type + " " +  pkt_name + ") { " + ret.first + "}\n";
  new_decls_ = ret.second;
}

template <class TransformType>
bool FuncTransformHandler<TransformType>::is_packet_func(const FunctionDecl * func_decl) const {
  // Not sure what we would get out of functions with zero args
  assert(func_decl->getNumParams() >= 1);
  std::cerr << "First parameter: " << func_decl->getParamDecl(0)->getType().getAsString() << std::endl;

  return func_decl->getNumParams() == 1
         and func_decl->getParamDecl(0)->getType().getAsString() == "Packet";
}

#endif  // FUNC_TRANSFORM_HANDLER_CC_
