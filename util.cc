#include "util.h"

#include <cstdlib>

#include <iostream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <exception>

std::string get_file_name(const int argc, const char ** argv) {
  std::string ret;
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " file_name " << std::endl;
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
  if (not ifs.good()) throw std::logic_error("Cannot read from " + file_name + ", maybe it doesn't exist?");
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()));
}

std::vector<std::string> split(const std::string & input, const std::string & regex_str) {
  std::regex regex_object(regex_str);
  std::sregex_token_iterator first{input.begin(), input.end(), regex_object, -1}, last;
  return {first, last};
}

std::string hash_string (const std::string & str) {
  return std::to_string(std::hash<std::string>()(str));
}

bool compare_after_removing_space(const std::string & s1, const std::string & s2) {
  auto s1_tmp = s1;
  auto s2_tmp = s2;
  s1_tmp.erase(std::remove_if(s1_tmp.begin(), s1_tmp.end(), isspace), s1_tmp.end());
  s2_tmp.erase(std::remove_if(s2_tmp.begin(), s2_tmp.end(), isspace), s2_tmp.end());
  return s1_tmp == s2_tmp;
}
