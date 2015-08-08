#ifndef UTIL_H_
#define UTIL_H_

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

#endif  // UTIL_H_
