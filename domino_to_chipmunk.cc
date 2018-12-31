#include "if_conversion_handler.h"
#include "ssa.h"
#include "expr_prop.h"
#include "partitioning.h"
#include "stateful_flanks.h"
#include "expr_flattener_handler.h"
#include "algebraic_simplifier.h"
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

// Both SinglePass and Transformer take a parameter pack as template
// arguments to allow additional arguments to the function carrying out
// the transformation (e.g., atom templates and pipeline width and depth in sketch_backend)
// Unfortunately, you can't provide a default value for such parameter packs
// As http://en.cppreference.com/w/cpp/language/template_parameters#Default_template_arguments says
// "Defaults can be specified for any kind of template parameter (type, non-type, or template), but not to parameter packs."
// This is my clunky workaround.
typedef Transformer<> DefaultTransformer;
typedef SinglePass<> DefaultSinglePass;

// Map to store factory for all passes
static  PassFactory all_passes;

void populate_passes() {
  // We need to explicitly call populate_passes instead of using an initializer list
  // to populate PassMap all_passes because initializer lists don't play well with move-only
  // types like unique_ptrs (http://stackoverflow.com/questions/9618268/initializing-container-of-unique-ptrs-from-initializer-list-fails-with-gcc-4-7)
  all_passes["cse"]               =[] () { return std::make_unique<FixedPointPass<CompoundPass, std::vector<DefaultTransformer>>>(std::vector<DefaultTransformer>({csi_transform, cse_transform})); };
  all_passes["redundancy_remover"]=[] () { return std::make_unique<FixedPointPass<DefaultSinglePass, DefaultTransformer>>(redundancy_remover_transform); };
  all_passes["array_validator"]  = [] () { return std::make_unique<DefaultSinglePass>(std::bind(& ArrayValidator::ast_visit_transform, ArrayValidator(), _1)); };
  all_passes["validator"]        = [] () { return std::make_unique<DefaultSinglePass>(std::bind(& Validator::ast_visit_transform, Validator(), _1)); };
  all_passes["int_type_checker"] = [] () { return std::make_unique<DefaultSinglePass>(std::bind(& IntTypeChecker::ast_visit_transform, IntTypeChecker(), _1)); };
  all_passes["desugar_comp_asgn"]= [] () { return std::make_unique<DefaultSinglePass>(std::bind(& DesugarCompAssignment::ast_visit_transform, DesugarCompAssignment(), _1)); };
  all_passes["if_converter"]     = [] () { return std::make_unique<DefaultSinglePass>(std::bind(& IfConversionHandler::transform, IfConversionHandler(), _1)); };
  all_passes["algebra_simplify"] = [] () { return std::make_unique<DefaultSinglePass>(std::bind(& AlgebraicSimplifier::ast_visit_transform, AlgebraicSimplifier(), _1)); };
  all_passes["bool_to_int"]      = [] () { return std::make_unique<DefaultSinglePass>(std::bind(& BoolToInt::ast_visit_transform, BoolToInt(), _1));};
  all_passes["expr_flattener"]   = [] () { return std::make_unique<FixedPointPass<DefaultSinglePass, DefaultTransformer>>(std::bind(& ExprFlattenerHandler::transform, ExprFlattenerHandler(), _1)); };
  all_passes["expr_propagater"]  = [] () { return std::make_unique<DefaultSinglePass>(expr_prop_transform); };
  all_passes["stateful_flanks"]  = [] () { return std::make_unique<DefaultSinglePass>(stateful_flank_transform); };
  all_passes["ssa"]              = [] () { return std::make_unique<DefaultSinglePass>(ssa_transform); };
  all_passes["echo"]             = [] () { return std::make_unique<DefaultSinglePass>(clang_decl_printer); };
  all_passes["gen_used_fields"]   = [] () { return std::make_unique<DefaultSinglePass>(gen_used_field_transform); };
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

void print_usage() {
  std::cerr << "Usage: domino <source_file> <atom_template> <pipeline depth> <pipeline width> <yes to run sketch preprocessor (no by default)> <comma-separated list of passes (optional)>" << std::endl;
  std::cerr << "List of passes: " << std::endl;
  std::cerr << all_passes_as_string(all_passes);
}

int main(int argc, const char **argv) {
  try {
    // Block out SIGINT, because we can't handle it properly
    signal(SIGINT, SIG_IGN);

    // Populate all passes
    populate_passes();

    // Default pass list
    const auto default_pass_list = "int_type_checker,desugar_comp_asgn,if_converter,algebra_simplify,array_validator,stateful_flanks,ssa,expr_propagater,expr_flattener,cse";

    if (argc >= 5) {
      // Get cmdline args
      const auto string_to_parse = file_to_str(std::string(argv[1]));
      const auto atom_template_file = std::string(argv[2]);
      const auto pipeline_depth = std::atoi(argv[3]);
      if (pipeline_depth <= 0) throw std::logic_error("Pipeline depth (" + std::string(argv[3]) + ") must be a positive integer");
      const auto pipeline_width = std::atoi(argv[4]);
      if (pipeline_width <= 0) throw std::logic_error("Pipeline width ("  + std::string(argv[4]) + ") must be a positive integer");
      const auto run_sketch_prepocessor = ((argc >= 6) and (std::string(argv[5]) == "yes")) ? true : false;
      const auto pass_list = (argc == 7) ? split(std::string(argv[6]), ","): split(default_pass_list, ",");

      if (argc > 7) {
        print_usage();
        return EXIT_FAILURE;
      }

      // add all preprocessing passes
      PassFunctorVector passes_to_run;
      for (const auto & pass_name : pass_list) passes_to_run.emplace_back(get_pass_functor(pass_name, all_passes));

      // add partitioning pass
      passes_to_run.emplace_back([pipeline_depth, pipeline_width] () { return std::make_unique<SinglePass<const uint32_t, const uint32_t>>(partitioning_transform, pipeline_depth, pipeline_width); } );

      // add the passes for the sketch preprocessor and backend
      if (run_sketch_prepocessor) passes_to_run.emplace_back([] () { return std::make_unique<DefaultSinglePass>(sketch_preprocessor); });
      passes_to_run.emplace_back([atom_template_file] () { return std::make_unique<SinglePass<const std::string>>(sketch_backend_transform, atom_template_file); });

      /// Process them one after the other
      std::cout << std::accumulate(passes_to_run.begin(), passes_to_run.end(), string_to_parse, [] (const auto & current_output, const auto & pass_functor __attribute__((unused)))
                                   { return (*pass_functor())(current_output); });

      return EXIT_SUCCESS;
    } else {
      print_usage();
      return EXIT_FAILURE;
    } 
  } catch (const std::exception & e) {
    std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
