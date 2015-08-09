#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

std::string get_file_name(const int argc, const char ** argv, const std::string & help_string) {
  std::string ret;
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " file_name " << std::endl;
    std::cerr << help_string << std::endl;
    exit(1);
    ret = "";
  } else  {
    ret = std::string(argv[1]);
  }
  return ret;
}

std::string file_to_str(const std::string & file_name) {
  // Taken from:
  // http://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
  std::ifstream ifs(file_name);
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()));
}

#endif  // UTIL_H_
