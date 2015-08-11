#include "if_conversion_handler.h"
#include "ssa.h"
#include "expr_prop.h"
#include "partitioning.h"
#include "stateful_flanks.h"
#include "expr_flattener_handler.h"
#include "strength_reducer.h"

#include <utility>
#include <iostream>
#include <set>
#include <string>
#include <functional>

#include "util.h"
#include "pkt_func_transform.h"
#include "single_pass.h"
#include "fixed_point_pass.h"

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;
using std::placeholders::_2;

// Vector of Transform functions, each transform function runs within a SinglePass
enum class TransformType { SINGLE, FIXED_POINT };
typedef  std::pair<TransformType, std::function<std::string(const clang::TranslationUnitDecl *)>> Transform;
typedef  std::vector<Transform> TransformVector;

int main(int argc, const char **argv) {
  // Get string that needs to be parsed
  const auto string_to_parse = file_to_str(get_file_name(argc, argv));

  // List out all passes
  TransformVector transforms = { std::make_pair(TransformType::SINGLE, std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1)),
                                 std::make_pair(TransformType::SINGLE, strength_reducer_transform),
                                 std::make_pair(TransformType::FIXED_POINT, std::bind(& ExprFlattenerHandler::transform, ExprFlattenerHandler(), _1)),
                                 std::make_pair(TransformType::SINGLE, expr_prop_transform),
                                 std::make_pair(TransformType::SINGLE, stateful_flank_transform),
                                 std::make_pair(TransformType::SINGLE, ssa_transform),
                                 std::make_pair(TransformType::SINGLE, partitioning_transform) };

  /// Process them one after the other
  std::cout << std::accumulate(transforms.begin(), transforms.end(), string_to_parse, [] (const auto & current_output, const auto & transform)
                               { return transform.first == TransformType::SINGLE ?
                                        SinglePass(transform.second)(current_output) :
                                        FixedPointPass(transform.second)(current_output); });

  return 0;
}
