#include "if_conversion_handler.h"
#include "prog_transforms.h"
#include "expr_flattener_handler.h"

#include <iostream>
#include <set>
#include <string>

#include "util.h"
#include "packet_variable_census.h"
#include "pkt_func_transform.h"
#include "single_pass.h"
#include "temp_file.hh"

static std::string help_string(""
"This allows us to rewrite if statements into ternary operators"
" and recursively get rid of all branches, from the innermost to"
" the outermost ones.");

int main(int argc, const char **argv) {
  // Get string that needs to be parsed
  const auto string_to_parse = file_to_str(get_file_name(argc, argv, help_string));

  // Parse file once and generate set of all packet variables
  const auto packet_var_set   = SinglePass<std::set<std::string>>(string_to_parse, packet_variable_census).output();

  // Parse file once and output if converted version
  IfConversionHandler if_conversion_handler(packet_var_set);
  const FuncBodyTransform if_converter = std::bind(& IfConversionHandler::transform, if_conversion_handler, std::placeholders::_1, std::placeholders::_2);
  const auto if_convert_output = SinglePass<std::string>(string_to_parse, std::bind(pkt_func_transform, std::placeholders::_1, if_converter)).output();

  // Parse file once again for strength reduction
  const auto strength_reduce_output = SinglePass<std::string>(if_convert_output, std::bind(pkt_func_transform, std::placeholders::_1, strength_reducer)).output();

  // Expression flattening, recurse to fixed point
  std::string old_output = strength_reduce_output;
  std::string new_output = "";
  while (true) {
    // Parse file once and generate set of all packet variables
    const auto packet_var_set = SinglePass<std::set<std::string>>(old_output, packet_variable_census).output();

    // Parse file once and output it after flattening
    ExprFlattenerHandler expr_flattener_handler(packet_var_set);
    const FuncBodyTransform expr_flattener = std::bind(& ExprFlattenerHandler::transform, expr_flattener_handler, std::placeholders::_1, std::placeholders::_2);
    new_output = SinglePass<std::string>(old_output, std::bind(pkt_func_transform, std::placeholders::_1, expr_flattener)).output();

    // Fixed point
    if (new_output == old_output) break;

    old_output = new_output;
  }

  // Parse file once and output it after propagating expressions
  const auto expr_prop_output = SinglePass<std::string>(new_output, std::bind(pkt_func_transform, std::placeholders::_1, expr_prop)).output();

  // Parse file once and output it after adding stateful flanks
  const auto packet_var_set_for_flanks = SinglePass<std::set<std::string>>(expr_prop_output, packet_variable_census).output();
  const FuncBodyTransform stateful_flank_converter = std::bind(stateful_flank_transform, std::placeholders::_1, std::placeholders::_2, packet_var_set_for_flanks);
  std::cout << SinglePass<std::string>(expr_prop_output, std::bind(pkt_func_transform, std::placeholders::_1, stateful_flank_converter)).output();

  return 0;
}
