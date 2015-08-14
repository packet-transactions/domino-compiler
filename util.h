#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

std::string get_file_name(const int argc, const char ** argv);

std::string file_to_str(const std::string & file_name);

#endif  // UTIL_H_
