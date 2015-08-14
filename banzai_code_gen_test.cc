/// Simple test program to test the Banzai code generator
#include "banzai_code_generator.h"

#include <exception>
#include <functional>

#include "third_party/temp_file.hh"
#include "third_party/system_runner.hh"

#include "config.h"
#include "util.h"
#include "compiler_pass.h"

using std::placeholders::_1;

int main(int argc, const char ** argv) {
  try {
    // Get string that needs to be parsed
    const auto string_to_parse = file_to_str(get_file_name(argc, argv));

    TransformVector transforms;
    transforms.emplace_back(std::make_unique<SinglePass>(std::bind(& BanzaiCodeGenerator::transform_translation_unit, BanzaiCodeGenerator(), _1)));

    TempFile banzai_file("/tmp/banzai_prog", ".cc");
    banzai_file.write(std::accumulate(transforms.begin(), transforms.end(), string_to_parse, [] (const auto & current_output, const auto & transform)
                                      { return (*transform)(current_output); }));

    /// Compiler banzai_file into a .o file
    TempFile object_file("/tmp/banzai_obj", ".o");
    run({GPLUSPLUS, "-std=c++14", "-pedantic", "-Wconversion", "-Wsign-conversion", "-Wall", "-Wextra", "-Weffc++", "-Werror", "-fno-default-inline", "-g", "-c", banzai_file.name(), "-fPIC", "-DPIC", "-o", object_file.name()});

    /// Turn that into a shared library
    UniqueFile library_file("./libbanzai", ".so");
    run({GPLUSPLUS, "-shared", "-o", library_file.name(), object_file.name()});

  } catch (const std::exception & e) {
   std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
  }
}
