#include "if_conversion_handler.h"
#include "ssa.h"
#include "expr_prop.h"
#include "partitioning.h"
#include "stateful_flanks.h"
#include "expr_flattener_handler.h"
#include "algebraic_simplifier.h"
#include "banzai_code_generator.h"
#include "p4_backend.h"
#include "bool_to_int.h"
#include "desugar_compound_assignment.h"
#include "int_type_checker.h"
#include "array_validator.h"
#include "validator.h"
#include "redundancy_remover.h"
#include "sketch_backend.h"
#include "cse.h"
#include "csi.h"
#include "gen_used_fields.h"

#include <csignal>

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
typedef std::function<std::unique_ptr<CompilerPass>(void)> PassFunctor;
typedef std::map<PassName, PassFunctor> PassFactory;
typedef std::vector<PassFunctor> PassFunctorVector;

// Map to store factory for all passes
static  PassFactory all_passes;

void populate_passes() {
  // We need to explicitly call populate_passes instead of using an initializer list
  // to populate PassMap all_passes because initializer lists don't play well with move-only
  // types like unique_ptrs (http://stackoverflow.com/questions/9618268/initializing-container-of-unique-ptrs-from-initializer-list-fails-with-gcc-4-7)
  all_passes["cse"]               =[] () { return std::make_unique<FixedPointPass<CompoundPass, std::vector<Transformer>>>(std::vector<Transformer>({csi_transform, cse_transform})); };
  all_passes["sketch_backend"]    =[] () { return std::make_unique<SinglePass>(sketch_backend_transform); };
  all_passes["redundancy_remover"]=[] () { return std::make_unique<FixedPointPass<SinglePass, Transformer>>(redundancy_remover_transform); };
  all_passes["array_validator"]  = [] () { return std::make_unique<SinglePass>(array_validator_transform); };
  all_passes["validator"]        = [] () { return std::make_unique<SinglePass>(std::bind(& Validator::ast_visit_transform, Validator(), _1)); };
  all_passes["int_type_checker"] = [] () { return std::make_unique<SinglePass>(int_type_checker_transform); };
  all_passes["desugar_comp_asgn"]= [] () { return std::make_unique<SinglePass>(std::bind(& DesugarCompAssignment::ast_visit_transform, DesugarCompAssignment(), _1)); };
  all_passes["if_converter"]     = [] () { return std::make_unique<SinglePass>(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1)); };
  all_passes["algebra_simplify"] = [] () { return std::make_unique<SinglePass>(std::bind(& AlgebraicSimplifier::ast_visit_transform, AlgebraicSimplifier(), _1)); };
  all_passes["bool_to_int"]      = [] () { return std::make_unique<SinglePass>(bool_to_int_transform); };
  all_passes["expr_flattener"]   = [] () { return std::make_unique<FixedPointPass<SinglePass, Transformer>>(std::bind(& ExprFlattenerHandler::transform, ExprFlattenerHandler(), _1)); };
  all_passes["expr_propagater"]  = [] () { return std::make_unique<SinglePass>(expr_prop_transform); };
  all_passes["stateful_flanks"]  = [] () { return std::make_unique<SinglePass>(stateful_flank_transform); };
  all_passes["ssa"]              = [] () { return std::make_unique<SinglePass>(ssa_transform); };
  all_passes["partitioning"]     = [] () { return std::make_unique<SinglePass>(partitioning_transform); };
  all_passes["banzai_source"]    = [] () { return std::make_unique<SinglePass>(std::bind(& BanzaiCodeGenerator::transform_translation_unit, BanzaiCodeGenerator(BanzaiCodeGenerator::CodeGenerationType::SOURCE), _1)); };
  all_passes["p4_source"]        = [] () { return std::make_unique<SinglePass>(std::bind(& P4CodeGenerator::transform_translation_unit, P4CodeGenerator(), _1)); };
  all_passes["banzai_binary"]    = [] () { return std::make_unique<SinglePass>(std::bind(& BanzaiCodeGenerator::transform_translation_unit, BanzaiCodeGenerator(BanzaiCodeGenerator::CodeGenerationType::BINARY), _1)); };
  all_passes["echo"]             = [] () { return std::make_unique<SinglePass>(clang_decl_printer); };
  all_passes["gen_used_fields"]   = [] () { return std::make_unique<SinglePass>(gen_used_field_transform); };
}

PassFunctor get_pass_functor(const std::string & pass_name, const PassFactory & pass_factory) {
  if (pass_factory.find(pass_name) != pass_factory.end()) {
    // This is a bit misleading because we don't have deep const correctness
    return pass_factory.at(pass_name);
  } else {
    throw std::logic_error("Unknown pass " + pass_name);
  }
}

std::string all_passes_as_string(const PassFactory & pass_factory) {
  std::string ret;
  for (const auto & pass_pair : pass_factory)
    ret += pass_pair.first + "\n";
  return ret;
}

int main(int argc, const char **argv) {
  try {
    // Block out SIGINT, because we can't handle it properly
    signal(SIGINT, SIG_IGN);

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
      PassFunctorVector passes_to_run;
      for (const auto & pass_name : pass_list) passes_to_run.emplace_back(get_pass_functor(pass_name, all_passes));

      /// Process them one after the other
      std::cout << std::accumulate(passes_to_run.begin(), passes_to_run.end(), string_to_parse, [] (const auto & current_output, const auto & pass_functor __attribute__((unused)))
                                   { return (*pass_functor())(current_output); });

      return EXIT_SUCCESS;
    }
  } catch (const std::exception & e) {
    std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
