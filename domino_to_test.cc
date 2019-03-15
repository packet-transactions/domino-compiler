#include <csignal>
#include "domino_to_test_code_gen.h"

#include <utility>
#include <iostream>
#include <set>
#include <string>
#include <functional>

#include "third_party/assert_exception.h"

#include "util.h"
#include "pkt_func_transform.h"
#include "compiler_pass.h"
#include <map>

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;
using std::placeholders::_2;

void print_usage() {
  std::cerr << "Usage: domino_to_rename_domino <source_file>" << std::endl;
}

int main(int argc, const char **argv) {
  try {
    // Block out SIGINT, because we can't handle it properly
    signal(SIGINT, SIG_IGN);

    if (argc == 2) {
      const auto string_to_parse = file_to_str(std::string(argv[1]));
      
      std::map<std::string,std::string> map;
      map["state_0"] = "state_hello";
      map["state_1"] = "state_hello1";
      map["state_2"] = "state_hello2";
      map["state_3"] = "state_hello3";
      map["state_4"] = "state_hello4";
      map["state_5"] = "state_hello5";
      auto rename_domino_code_generator = SinglePass<>(std::bind(& RenameDominoCodeGenerator::ast_visit_transform_mutator,
                                                  RenameDominoCodeGenerator(map),_1));
      
      std::string sketch_program = rename_domino_code_generator(string_to_parse);
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
