#include "if_conversion_handler.h"
#include "ssa.h"
#include "expr_prop.h"
#include "partitioning.h"
#include "stateful_flanks.h"
#include "expr_flattener_handler.h"
#include "prog_transforms.h"

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

  // Variable to store identifier sets, whenever required
  std::set<std::string> id_set;

  // Parse file once and generate identifier set
  id_set   = SinglePass<std::set<std::string>>(identifier_census)(string_to_parse);

  // Parse file once and output if converted version
  IfConversionHandler if_conversion_handler(id_set);
  const FuncBodyTransform if_converter = std::bind(& IfConversionHandler::transform, if_conversion_handler, _1, _2);
  const auto if_convert_output = SinglePass<std::string>(std::bind(pkt_func_transform, _1, if_converter))(string_to_parse);

  // Parse file once again for strength reduction
  const auto strength_reduce_output = SinglePass<std::string>(std::bind(pkt_func_transform, _1, strength_reducer))(if_convert_output);

  // Expression flattening, recurse to fixed point
  const auto & expr_flat_output = FixedPointPass<std::string>(ExprFlattenerHandler::transform)(strength_reduce_output);

  TransformVector transforms = { expr_prop_transform, stateful_flank_transform, ssa_transform, partitioning_transform };
  std::cout << std::accumulate(transforms.begin(), transforms.end(), expr_flat_output, [] (const auto & current_output, const auto & transform)
                               { return SinglePass<std::string>(transform)(current_output); });

  return 0;
}
