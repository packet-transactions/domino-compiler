#include <csignal>
#include "chipmunk_deadcode_generator.h"

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

void print_usage() {
  std::cerr << "Usage: domino_to_Deaddomino <source_file>" << std::endl;
}

int main(int argc, const char **argv) {
  try {
    // Block out SIGINT, because we can't handle it properly
    signal(SIGINT, SIG_IGN);

    if (argc == 2){
      const auto string_to_parse = file_to_str(std::string(argv[1]));

      auto chipmunk_deadcode_generator = SinglePass<>(std::bind(& ChipmunkDeadcodeGenerator::ast_visit_transform,
                                                  ChipmunkDeadcodeGenerator(), _1));

      std::string sketch_program = chipmunk_deadcode_generator(string_to_parse);
      std::cout << sketch_program << std::endl;

      return EXIT_SUCCESS;
    }
    else {
      print_usage();
      return EXIT_FAILURE;
    }
  } catch (const std::exception & e) {
    std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
