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
#include <regex>

#include "util.h"
#include "pkt_func_transform.h"
#include "compiler_pass.h"

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;
using std::placeholders::_2;

/// Split string based on another string used as delimiter
/// using C++11's regex_token_iterator
/// Based on http://en.cppreference.com/w/cpp/regex/regex_token_iterator
/// and http://stackoverflow.com/a/9437426/1152801
std::vector<std::string> split(const std::string & input, const std::string & regex_str) {
  std::regex regex_object(regex_str);
  std::sregex_token_iterator first{input.begin(), input.end(), regex_object, -1}, last;
  return {first, last};
}

int main(int argc, const char **argv) {
  try {
    // Get string that needs to be parsed and pass list
    std::string string_to_parse = "";
    std::vector<std::string> pass_list;
    if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " file_name comma-separated pass list (if_converter, strength_reducer, expr_flattener, expr_propagater, stateful_flanks, ssa, partitioning) \n";
      exit(1);
    } else {
      string_to_parse = std::string(argv[1]);
      pass_list = split(std::string(argv[2]), ",");
    }

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
  } catch (const std::exception & e) {
    std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
  }
  return 0;
}
