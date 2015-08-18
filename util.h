#ifndef UTIL_H_
#define UTIL_H_

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

/// Get file name from command line, exit if not available
std::string get_file_name(const int argc, const char ** argv);

/// Read a file's contents into a string
std::string file_to_str(const std::string & file_name);

/// Split string based on another string used as delimiter
/// using C++11's regex_token_iterator
/// Based on http://en.cppreference.com/w/cpp/regex/regex_token_iterator
/// and http://stackoverflow.com/a/9437426/1152801
std::vector<std::string> split(const std::string & input, const std::string & regex_str);

/// Hash string into unique ID and turn that into a string
std::string hash_string (const std::string & str);

/// Compare two strings after removing all forms of whitespace
bool compare_after_removing_space(const std::string & s1, const std::string & s2);

#endif  // UTIL_H_
