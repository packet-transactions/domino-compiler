#include "if_conversion_handler.h"
#include "ssa.h"
#include "expr_prop.h"
#include "partitioning.h"
#include "stateful_flanks.h"
#include "expr_flattener_handler.h"
#include "strength_reducer.h"

#include <iostream>
#include <set>
#include <string>
#include <functional>

#include "util.h"
#include "identifier_census.h"
#include "pkt_func_transform.h"
#include "single_pass.h"
#include "fixed_point_pass.h"

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;
using std::placeholders::_2;

// Vector of Transform functions, each transform function runs within a SinglePass
typedef  std::function<std::string(const clang::TranslationUnitDecl *)> Transform;
typedef  std::vector<Transform> TransformVector;

int main(int argc, const char **argv) {
  // Get string that needs to be parsed
  const auto string_to_parse = file_to_str(get_file_name(argc, argv));

  // Parse file once and output if converted version
  const auto if_convert_output = SinglePass<std::string>(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1))(string_to_parse);

  // Parse file once again for strength reduction
  const auto strength_reduce_output = SinglePass<std::string>(strength_reducer_transform)(if_convert_output);

  // Expression flattening, recurse to fixed point
  const auto & expr_flat_output = FixedPointPass<std::string>(ExprFlattenerHandler::transform)(strength_reduce_output);

  TransformVector transforms = { expr_prop_transform, stateful_flank_transform, ssa_transform, partitioning_transform };
  std::cout << std::accumulate(transforms.begin(), transforms.end(), expr_flat_output, [] (const auto & current_output, const auto & transform)
                               { return SinglePass<std::string>(transform)(current_output); });

  return 0;
}
