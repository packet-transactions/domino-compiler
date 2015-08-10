#include "if_conversion_handler.h"
#include "prog_transforms.h"
#include "expr_flattener_handler.h"

#include <iostream>
#include <set>
#include <string>

#include "util.h"
#include "identifier_census.h"
#include "pkt_func_transform.h"
#include "single_pass.h"
#include "temp_file.hh"

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;
using std::placeholders::_2;

int main(int argc, const char **argv) {
  // Get string that needs to be parsed
  const auto string_to_parse = file_to_str(get_file_name(argc, argv));

  // Variable to store identifier sets, whenever required
  std::set<std::string> id_set;

  // Parse file once and generate identifier set
  id_set   = SinglePass<std::set<std::string>>(string_to_parse, identifier_census).output();

  // Parse file once and output if converted version
  IfConversionHandler if_conversion_handler(id_set);
  const FuncBodyTransform if_converter = std::bind(& IfConversionHandler::transform, if_conversion_handler, _1, _2);
  const auto if_convert_output = SinglePass<std::string>(string_to_parse, std::bind(pkt_func_transform, _1, if_converter)).output();

  // Parse file once again for strength reduction
  const auto strength_reduce_output = SinglePass<std::string>(if_convert_output, std::bind(pkt_func_transform, _1, strength_reducer)).output();

  // Expression flattening, recurse to fixed point
  std::string old_output = strength_reduce_output;
  std::string new_output = "";
  while (true) {
    // Parse file once and generate identifier set
    id_set = SinglePass<std::set<std::string>>(old_output, identifier_census).output();

    // Parse file once and output it after flattening
    ExprFlattenerHandler expr_flattener_handler(id_set);
    const FuncBodyTransform expr_flattener = std::bind(& ExprFlattenerHandler::transform, expr_flattener_handler, _1, _2);
    new_output = SinglePass<std::string>(old_output, std::bind(pkt_func_transform, _1, expr_flattener)).output();

    // Fixed point
    if (new_output == old_output) break;

    old_output = new_output;
  }

  // Parse file once and output it after propagating expressions
  const auto expr_prop_output = SinglePass<std::string>(new_output, std::bind(pkt_func_transform, _1, expr_prop)).output();

  // Parse file once and output it after adding stateful flanks
  id_set = SinglePass<std::set<std::string>>(expr_prop_output, identifier_census).output();
  const FuncBodyTransform stateful_flank_converter = std::bind(stateful_flank_transform, _1, _2, id_set);
  const auto flank_output = SinglePass<std::string>(expr_prop_output, std::bind(pkt_func_transform, _1, stateful_flank_converter)).output();

  // Parse file once and output it after Stateful SSA
  id_set = SinglePass<std::set<std::string>>(flank_output, identifier_census).output();
  const FuncBodyTransform ssa_converter = std::bind(ssa_transform, _1, _2, id_set);
  const auto ssa_output = SinglePass<std::string>(flank_output, std::bind(pkt_func_transform, _1, ssa_converter)).output();

  // Generate dependency graph after condensing strongly connected components
  // Use the condensed DAG to partition code
  id_set = SinglePass<std::set<std::string>>(ssa_output, identifier_census).output();
  std::cout << SinglePass<std::string>(ssa_output, std::bind(partitioning_transform, _1, id_set)).output();

  return 0;
}
