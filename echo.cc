#include <iostream>

#include "util.h"
#include "new_single_pass.h"
#include "clang_utility_functions.h"

static std::string help_string(""
"Driver program for single pass program that takes"
"in a file and echoes it out as such");

int main(int argc, const char **argv) {
  // Parse file once and output it as such. This is just a test program
  std::cout << NewSinglePass<std::string>(get_file_name(argc, argv, help_string), help_string, clang_decl_printer).output();
}
