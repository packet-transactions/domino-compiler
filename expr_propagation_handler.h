#ifndef EXPR_PROPAGATION_HANDLER_H_ 
#define EXPR_PROPAGATION_HANDLER_H_ 

#include <utility>
#include <string>
#include <vector>
#include <set>
#include "clang/AST/AST.h"

class ExprPropagationHandler {
 public:
  /// Constructor
  ExprPropagationHandler() {}

  /// Transform function
  std::pair<std::string, std::vector<std::string>> transform(const clang::Stmt * function_body, const std::string & pkt_name) const;
};

#endif  // EXPR_PROPAGATION_HANDLER_H_
