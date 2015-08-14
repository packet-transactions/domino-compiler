/// Simple test program to test the Banzai code generator
#include "banzai_code_generator.h"

#include <functional>

#include "util.h"
#include "compiler_pass.h"

using std::placeholders::_1;

int main(int argc, const char ** argv) {
  // Get string that needs to be parsed
  const auto string_to_parse = file_to_str(get_file_name(argc, argv));

  TransformVector transforms;
  transforms.emplace_back(std::make_unique<SinglePass>(std::bind(& BanzaiCodeGenerator::transform_translation_unit, BanzaiCodeGenerator(), _1)));

  std::cout << std::accumulate(transforms.begin(), transforms.end(), string_to_parse, [] (const auto & current_output, const auto & transform)
                               { return (*transform)(current_output); });


}
