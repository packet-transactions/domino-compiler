/// Simple test program to test the Banzai code generator
#include "banzai_code_generator.h"

#include <exception>
#include <functional>

#include "util.h"
#include "compiler_pass.h"

using std::placeholders::_1;

int main(int argc, const char ** argv) {
  try {
    // Get string that needs to be parsed
    const auto string_to_parse = file_to_str(get_file_name(argc, argv));

    TransformVector transforms;
    transforms.emplace_back(std::make_unique<SinglePass>(std::bind(& BanzaiCodeGenerator::transform_translation_unit, BanzaiCodeGenerator(BanzaiCodeGenerator::CodeGenerationType::BINARY), _1)));

    // The BanzaiCodeGenerator::transform_translation_unit method takes in a domino program
    // and turns it into a shared library of banzai atoms that can be run by banzai
    // The output, as a result, is in binary, which means it could bork the terminal.
    std::cout << (std::accumulate(transforms.begin(), transforms.end(), string_to_parse, [] (const auto & current_output, const auto & transform)
                                  { return (*transform)(current_output); }));
  } catch (const std::exception & e) {
    std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
  }
}
