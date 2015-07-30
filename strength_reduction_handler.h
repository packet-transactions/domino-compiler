#ifndef STRENGTH_REDUCTION_HANDLER_H_ 
#define STRENGTH_REDUCTION_HANDLER_H_ 

#include <utility>
#include <string>
#include <vector>
#include <set>
#include "clang/AST/AST.h"

class StrengthReductionHandler {
 public:
  /// Constructor
  StrengthReductionHandler() {}

  /// Transform function
  std::pair<std::string, std::vector<std::string>> transform(const clang::Stmt * function_body, const std::string & pkt_name) const;
};

#endif  // STRENGTH_REDUCTION_HANDLER_H_
