#include "if_conversion_handler.h"
#include "ssa.h"
#include "expr_prop.h"
#include "partitioning.h"
#include "stateful_flanks.h"
#include "expr_flattener_handler.h"
#include "strength_reducer.h"

#include <memory>
#include <utility>
#include <iostream>
#include <set>
#include <string>
#include <functional>

#include "util.h"
#include "pkt_func_transform.h"
#include "compiler_pass.h"

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;
using std::placeholders::_2;

// Vector of Transform functions, each transform function runs within a CompilerPass
typedef  std::vector<std::unique_ptr<CompilerPass>> TransformVector;

int main(int argc, const char **argv) {
  // Get string that needs to be parsed
  const auto string_to_parse = file_to_str(get_file_name(argc, argv));

  // add all passes
  // Unfortunately, we can't use a simpler initializer list because initializer lists
  // use the copy constructor, while unique_ptr's are move-only
  // http://stackoverflow.com/questions/9618268/initializing-container-of-unique-ptrs-from-initializer-list-fails-with-gcc-4-7
  // http://stackoverflow.com/questions/8468774/can-i-list-initialize-a-vector-of-move-only-type/8469002#8469002
  // http://stackoverflow.com/questions/8193102/initializer-list-and-move-semantics
  TransformVector transforms;
  transforms.emplace_back(std::make_unique<SinglePass>(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1)));
  transforms.emplace_back(std::make_unique<SinglePass>(strength_reducer_transform));
  transforms.emplace_back(std::make_unique<FixedPointPass>(std::bind(& ExprFlattenerHandler::transform, ExprFlattenerHandler(), _1)));
  transforms.emplace_back(std::make_unique<SinglePass>(expr_prop_transform));
  transforms.emplace_back(std::make_unique<SinglePass>(stateful_flank_transform));
  transforms.emplace_back(std::make_unique<SinglePass>(ssa_transform));
  transforms.emplace_back(std::make_unique<SinglePass>(partitioning_transform));

  /// Process them one after the other
  std::cout << std::accumulate(transforms.begin(), transforms.end(), string_to_parse, [] (const auto & current_output, const auto & transform)
                               { return (*transform)(current_output); });

  return 0;
}
