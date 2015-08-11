#ifndef FIXED_POINT_PASS_H_
#define FIXED_POINT_PASS_H_

#include <string>
#include <functional>

#include "clang/AST/AST.h"

#include "single_pass.h"

// Run a SinglePass repeatedly until the output converges to a fixed point
template <class OutputType>
class FixedPointPass {
 public:
  typedef std::function<OutputType(const clang::TranslationUnitDecl *)> Transformer;

  /// Initialize a FixedPointPass
  FixedPointPass(const Transformer & t_transformer)
      : transformer_(t_transformer) {}

  /// Retrieve output
  OutputType operator() (const std::string & string_to_parse) {
    std::string old_output = string_to_parse;
    std::string new_output = "";
    while (true) {
      new_output = SinglePass<std::string>(transformer_)(old_output);
      if (new_output == old_output) break;
      old_output = new_output;
    }
    return new_output;
  }

 private:
  /// Store t_transformer for future use
  Transformer transformer_;
};

#endif  // FIXED_POINT_PASS_H_
