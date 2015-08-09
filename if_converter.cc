#include "if_conversion_handler.h"
#include "prog_transforms.h"

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
  // Parse file once and generate set of all packet variables
  const auto packet_var_set   = SinglePass<std::set<std::string>>(get_file_name(argc, argv, help_string), packet_variable_census).output();

  // Parse file once and output if converted version
  IfConversionHandler if_conversion_handler(packet_var_set);
  const FuncBodyTransform if_converter = std::bind(& IfConversionHandler::transform, if_conversion_handler, std::placeholders::_1, std::placeholders::_2);
  TempFile tmp1("if_convert", ".c");
  tmp1.write(SinglePass<std::string>(get_file_name(argc, argv, help_string), std::bind(pkt_func_transform, std::placeholders::_1, if_converter)).output());

  // Parse file once again for strength reduction
  std::cout << SinglePass<std::string>(tmp1.name(), std::bind(pkt_func_transform, std::placeholders::_1, strength_reducer)).output();

  return 0;
}
