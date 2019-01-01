#include <csignal>
#include "chipmunk_code_generator.h"

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
  std::cerr << "Usage: domino_to_chipmunk <source_file>" << std::endl;
}

int main(int argc, const char **argv) {
  try {
    // Block out SIGINT, because we can't handle it properly
    signal(SIGINT, SIG_IGN);

    if (argc == 2) {
      const auto string_to_parse = file_to_str(std::string(argv[1]));

      std::string file_org = "/*"; //original code
      std::string target_str = "func"; //find the void function
      
      std::string::size_type location;
      location = string_to_parse.find(target_str);

      file_org += string_to_parse.substr(location,string_to_parse.length()-location);
      file_org += "*/";

      auto chipmunk_code_generator = SinglePass<>(std::bind(& ChipmunkCodeGenerator::ast_visit_transform, ChipmunkCodeGenerator(), _1));
      std::string new_program = chipmunk_code_generator(string_to_parse);
      location = new_program.find(target_str);
      new_program = new_program.substr(location, new_program.rfind('}')-location);
      
      new_program = "|StateAndPacket| program (|StateAndPacket| state_and_packet)" + new_program;
      new_program += " return state_and_packet;\n";
      new_program +='}';
      
      std::cout << file_org << std::endl;
      std::cout << new_program << std::endl;
      
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
