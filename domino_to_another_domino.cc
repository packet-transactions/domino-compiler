#include <csignal>
#include "chipmunk_deadcode_generator.h"
#include "chipmunk_anotherDomino_generator.h"


#include <utility>
#include <iostream>
#include <set>
#include <string>
#include <functional>

#include <fstream>

#include "third_party/assert_exception.h"

#include "util.h"
#include "pkt_func_transform.h"
#include "compiler_pass.h"
#include <stdlib.h>

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;

void print_usage() {
  std::cerr << "Usage: domino_to_anotherDomino <source_file>" << std::endl;
}

int main(int argc, const char **argv) {
  try {
    // Block out SIGINT, because we can't handle it properly
    signal(SIGINT, SIG_IGN);

    if (argc == 2){
      int num_of_transformed_file = 1;
      std::string domino_file_name = std::string(argv[1]);
      std::size_t pos_begin = domino_file_name.rfind("/");
      std::size_t pos_end = domino_file_name.find(".");
      domino_file_name = domino_file_name.substr(pos_begin+1,pos_end-pos_begin-1);

      while(num_of_transformed_file != 101){
        

        const auto string_to_parse = file_to_str(std::string(argv[1]));
        ChipmunkAnotherdominoGenerator AnotherDomino;
        auto chipmunk_another_domino_generator = SinglePass<>(std::bind(& ChipmunkAnotherdominoGenerator::ast_visit_transform,
                                                    AnotherDomino, _1));
        int count = 0;
        std::string sketch_program = chipmunk_another_domino_generator(string_to_parse);
        while (count < 101){
            count++;
            //random_num is to record which execution to take
            int random_num = rand() % 7 + 1;
            if (random_num >=1 && random_num <=3){
              ChipmunkAnotherdominoGenerator another_domino;
              another_domino.round = count;
              another_domino.rand = random_num;
              auto chipmunk_another_domino_generator = SinglePass<>(std::bind(& ChipmunkAnotherdominoGenerator::ast_visit_transform,
                                                    another_domino, _1));
              sketch_program = chipmunk_another_domino_generator(sketch_program);
            }else{
              ChipmunkDeadcodeGenerator domino_with_deadcode;
              domino_with_deadcode.rand = random_num;
              auto chipmunk_deadcode_generator = SinglePass<>(std::bind(& ChipmunkDeadcodeGenerator::ast_visit_transform,
                                                    domino_with_deadcode, _1));
              sketch_program = chipmunk_deadcode_generator(sketch_program);
            }    
        }
        std::string filename = "/tmp/" + domino_file_name + "_equivalent_" + std::to_string(num_of_transformed_file) + ".c";
        std::ofstream myfile;
        myfile.open (filename.c_str());
        myfile << sketch_program;
        myfile.close();
        num_of_transformed_file++;
      }        

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
