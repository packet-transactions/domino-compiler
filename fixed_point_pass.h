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

  /// Run a FixedPointPass on a given string
  /// until it converges to a fixed point
  FixedPointPass(const std::string & string_to_parse,
                 const Transformer & t_transformer) {
    std::string old_output = string_to_parse;
    std::string new_output = "";
    while (true) {
      new_output = SinglePass<std::string>(old_output, t_transformer).output();
      if (new_output == old_output) break;
      old_output = new_output;
    }
    output_ = new_output;
  }

  /// Retrieve output
  auto output() const { return output_; }

 private:
  /// Store output in a string
  std::string output_ = "";
};

#endif  // FIXED_POINT_PASS_H_
