#ifndef FUNC_TRANSFORM_HANDLER_H_
#define FUNC_TRANSFORM_HANDLER_H_

#include <string>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

template <class TransformType, typename ...Fargs>
class FuncTransformHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  /// Constructor
  FuncTransformHandler(Fargs... args) : transformer_(args...) {}

  /// Callback whenever there's a match
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;

  /// Get output
  auto output() const { return std::make_pair(out_str_, new_decls_); }

 private:
  /// The transformer itself
  TransformType transformer_ = {};

  /// Output stream
  std::string out_str_ = "";

  /// New declarations
  std::vector<std::string> new_decls_ = {};
};

#endif  // FUNC_TRANSFORM_HANDLER_H_
