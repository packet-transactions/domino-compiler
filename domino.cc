#include "if_conversion_handler.h"
#include "ssa.h"
#include "expr_prop.h"
#include "partitioning.h"
#include "stateful_flanks.h"
#include "expr_flattener_handler.h"
#include "strength_reducer.h"
#include "banzai_code_generator.h"
#include "desugar_compound_assignment.h"
#include "int_type_checker.h"

#include <utility>
#include <iostream>
#include <set>
#include <string>
#include <functional>

#include "third_party/assert_exception.h"

#include "util.h"
#include "pkt_func_transform.h"
#include "compiler_pass.h"

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;
using std::placeholders::_2;

// Convenience typedefs
typedef std::string PassName;
typedef std::map<PassName, std::unique_ptr<CompilerPass>> PassMap;

// Map to store all passes
static  PassMap all_passes;

void populate_passes() {
  // We need to explicitly call populate_passes instead of using an initializer list
  // to populate PassMap all_passes because initializer lists don't play well with move-only
  // types like unique_ptrs (http://stackoverflow.com/questions/9618268/initializing-container-of-unique-ptrs-from-initializer-list-fails-with-gcc-4-7)
  all_passes["int_type_checker"] = std::make_unique<SinglePass>(int_type_checker_transform);
  all_passes["desugar_comp_asgn"]= std::make_unique<SinglePass>(desugar_compound_assignment_transform);
  all_passes["if_converter"]     = std::make_unique<SinglePass>(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1));
  all_passes["strength_reducer"] = std::make_unique<SinglePass>(strength_reducer_transform);
  all_passes["expr_flattener"]   = std::make_unique<FixedPointPass>(std::bind(& ExprFlattenerHandler::transform, ExprFlattenerHandler(), _1));
  all_passes["expr_propagater"]  = std::make_unique<SinglePass>(expr_prop_transform);
  all_passes["stateful_flanks"]  = std::make_unique<SinglePass>(stateful_flank_transform);
  all_passes["ssa"]              = std::make_unique<SinglePass>(ssa_transform);
  all_passes["partitioning"]     = std::make_unique<SinglePass>(partitioning_transform);
  all_passes["banzai_source"]    = std::make_unique<SinglePass>(std::bind(& BanzaiCodeGenerator::transform_translation_unit, BanzaiCodeGenerator(BanzaiCodeGenerator::CodeGenerationType::SOURCE), _1));
  all_passes["banzai_binary"]    = std::make_unique<SinglePass>(std::bind(& BanzaiCodeGenerator::transform_translation_unit, BanzaiCodeGenerator(BanzaiCodeGenerator::CodeGenerationType::BINARY), _1));
  all_passes["echo"]             = std::make_unique<SinglePass>(clang_decl_printer);
  all_passes["gen_pkt_fields"]   = std::make_unique<SinglePass>(gen_pkt_fields);
}

CompilerPass * get_pass(const std::string & pass_name, const PassMap & pass_map) {
  if (pass_map.find(pass_name) != pass_map.end()) {
    // This is a bit misleading because we don't have deep const correctness
    return pass_map.at(pass_name).get();
  } else {
    throw std::logic_error("Unknown pass " + pass_name);
  }
}

std::string all_passes_as_string(const PassMap & pass_map) {
  std::string ret;
  for (const auto & pass_pair : pass_map)
    ret += pass_pair.first + "\n";
  return ret;
}

int main(int argc, const char **argv) {
  try {
    // Populate all passes
    populate_passes();

    // Get string that needs to be parsed and pass list
    std::string string_to_parse = "";
    std::vector<std::string> pass_list;
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <source_file> <comma-separated list of passes given below>" << std::endl;
      std::cerr << all_passes_as_string(all_passes);
      return EXIT_FAILURE;
    } else {
      string_to_parse = file_to_str(std::string(argv[1]));
      pass_list = split(std::string(argv[2]), ",");

      // add all user-requested passes in the same order that the user specified
      TransformVector transforms;
      for (const auto & pass_name : pass_list) transforms.emplace_back(get_pass(pass_name, all_passes));

      /// Process them one after the other
      std::cout << std::accumulate(transforms.begin(), transforms.end(), string_to_parse, [] (const auto & current_output, const auto & transform)
                                   { return (*transform)(current_output); });

      return EXIT_SUCCESS;
    }
  } catch (const std::exception & e) {
    std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
