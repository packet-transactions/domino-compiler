#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "domino_to_group_domino_code_gen.h"
#include "map_size_collection.h"
#include <csignal>

#include <functional>
#include <set>
#include <utility>

#include "third_party/assert_exception.h"

#include "compiler_pass.h"
#include "pkt_func_transform.h"
#include "util.h"

// For the _1, and _2 in std::bind
// (Partial Function Application)
using std::placeholders::_1;

std::string file_to_str(const std::string &file_name) {
  // Taken from:
  // http://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
  std::ifstream ifs(file_name);
  if (not ifs.good())
    throw std::logic_error("Cannot read from " + file_name +
                           ", maybe it doesn't exist?");
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()));
}

// Generate all permutations
void Permutations(std::vector<int> origin_list,
                  std::vector<std::vector<int>> &permutation) {
  while (next_permutation(origin_list.begin(), origin_list.end())) {
    permutation.push_back(origin_list);
  }
}

// Sort all permutations
void sort_list(std::vector<std::vector<int>> &permutation_list,
               unsigned int group_size) {
  // Sort each group member by ascending order
  for (auto &perm : permutation_list) {
    for (unsigned int j = 0; j < perm.size(); j += group_size) {
      sort(perm.begin() + j, perm.begin() + j + group_size);
    }
  }
  // Sort the number of each group
  for (unsigned int i = 0; i != permutation_list.size(); i++) {
    for (unsigned int j = 0; j < permutation_list[i].size() - group_size;
         j = j + group_size) {
      if (permutation_list[i][j] > permutation_list[i][j + group_size])
        for (unsigned int k = j; k != j + group_size; k++) {
          int tmp = permutation_list[i][k];
          permutation_list[i][k] = permutation_list[i][k + group_size];
          permutation_list[i][k + group_size] = tmp;
        }
    }
  }
  return;
}

// Whether two vectors are the same
int is_same(std::vector<int> a, std::vector<int> b) {
  for (unsigned int i = 0; i != a.size(); i++) {
    if (a[i] != b[i])
      return 0;
  }
  return 1;
}

// Whether this vector has appeared in the vector<vector>
int is_in_vector(std::vector<int> arr,
                 std::vector<std::vector<int>> &permutation_without_repeat) {
  for (unsigned int i = 0; i != permutation_without_repeat.size(); i++) {
    if (is_same(arr, permutation_without_repeat[i]))
      return 1;
  }
  return 0;
}

// Delete the repeated member in the list
void del_repeat(std::vector<std::vector<int>> permutation_list,
                std::vector<std::vector<int>> &permutation_without_repeat) {
  for (unsigned int i = 0; i != permutation_list.size(); i++) {
    if (i == 0)
      permutation_without_repeat.push_back(permutation_list[i]);
    else {
      if (!is_in_vector(permutation_list[i], permutation_without_repeat))
        permutation_without_repeat.push_back(permutation_list[i]);
    }
  }
}

// generate all combinations
void combination_generator(std::vector<int> arr, std::vector<int> data,
                           unsigned int start, unsigned int end,
                           unsigned int index, unsigned int r,
                           std::vector<std::vector<int>> &res) {
  if (index == r) {
    res.push_back(data);
    return;
  }
  unsigned int i = start;
  while (i <= end && end - i + 1 >= r - index) {
    data[index] = arr[i];
    combination_generator(arr, data, i + 1, end, index + 1, r, res);
    i++;
  }
}

void group_collection(int total_number, unsigned int group_size,
                      std::vector<std::vector<std::vector<int>>> &group) {
  std::vector<int> arr;
  // Fill in arr
  for (int i = 0; i != total_number; i++) {
    arr.push_back(i);
  }
  // Pay attention to the case where the group_size is 1
  if (group_size == 1) {
    std::vector<std::vector<int>> group_num;
    for (unsigned int j = 0; j != arr.size(); j++) {
      std::vector<int> each_group;
      each_group.push_back(arr[j]);
      group_num.push_back(each_group);
    }
    group.push_back(group_num);
    return;
  }
  for (unsigned int i = 0; i <= arr.size() / group_size; i++) {
    if (i == 0) {
      std::vector<std::vector<int>> group_num;
      for (unsigned int j = 0; j != arr.size(); j++) {
        std::vector<int> each_group;
        each_group.push_back(arr[j]);
        group_num.push_back(each_group);
      }
      group.push_back(group_num);
      continue;
    }
    std::vector<std::vector<int>> res;
    std::vector<int> data;
    // num_to_pick means how many state vars to pick
    unsigned int num_to_pick = group_size * i; // this is a for loop
    for (unsigned int j = 0; j != num_to_pick; j++) {
      data.push_back(0);
    }
    // generate combination
    combination_generator(arr, data, 0, (unsigned int)(arr.size() - 1), 0,
                          num_to_pick, res);
    for (unsigned int p = 0; p != res.size(); p++) {
      std::vector<std::vector<int>> permutation;
      std::vector<std::vector<int>> permutation_without_repeat;
      // generate permutation
      Permutations(res[p], permutation);
      // generate sort_list
      sort_list(permutation, group_size);
      // delete the repeated member
      del_repeat(permutation, permutation_without_repeat);

      std::vector<std::vector<int>> group_num;
      std::vector<int> each_group;
      // Print out result
      for (unsigned int k = 0; k != permutation_without_repeat.size(); k++) {
        // group_num store the member in each group
        std::vector<std::vector<int>> group_num;
        // Print the member in Group of size 1
        for (unsigned int index = 0; index != arr.size(); index++) {
          // if the member of arr have not appear in
          // permutation_without_repeat[k]
          if (find(permutation_without_repeat[k].begin(),
                   permutation_without_repeat[k].end(),
                   arr[index]) == permutation_without_repeat[k].end()) {
            std::vector<int> each_group;
            each_group.push_back(arr[index]);
            group_num.push_back(each_group);
          }
        }
        for (unsigned int index = 0;
             index != permutation_without_repeat[k].size();
             index = index + group_size) {
          std::vector<int> each_group;
          for (unsigned int num = index; num != index + group_size; num++)
            each_group.push_back(permutation_without_repeat[k][num]);
          group_num.push_back(each_group);
        }
        group.push_back(group_num);
      }
    }
  }
}

void generate_map(std::vector<std::vector<int>> vec,
                  std::map<std::string, std::string> &state_to_group) {
  for (unsigned int i = 0; i != vec.size(); i++) {
    for (unsigned int j = 0; j != vec[i].size(); j++) {
      std::string str1 = "state_" + std::to_string(vec[i][j]);
      std::string str2 =
          "state_group_" + std::to_string(i) + "_state_" + std::to_string(j);
      state_to_group[str1] = str2;
    }
  }
}

int main(int argc, const char **argv) {
  unsigned int group_size;
  // total_number means how many state vars
  int total_number;
  std::string string_to_parse;
  if (argc == 3) {
    string_to_parse = file_to_str(std::string(argv[1]));
    group_size = (unsigned int) std::stoul(argv[2]);
  } else {
    std::cerr << "Usage: grouper <source file> <group_size(1 or 2)> "
              << std::endl;
    return EXIT_FAILURE;
  }
  //get the filename
  std::string src_filename = std::string(argv[1]);
  src_filename = src_filename.substr(src_filename.rfind('/')+1,src_filename.rfind('.')-src_filename.rfind('/')-1);

  std::size_t found = 0;
  int max_state_var_num = 0;
  while (found != std::string::npos) {
    found = string_to_parse.find("state_", found + 1);
    if (found != std::string::npos) {
      found = string_to_parse.find("_", found + 1);
      if (string_to_parse[found + 1] - '0' > max_state_var_num)
        max_state_var_num = string_to_parse[found + 1] - '0';
    }
  }
  total_number = max_state_var_num + 1;
/*  auto map_size_generator = SinglePass<>(
         std::bind(&MapSizeCodeGenerator::map_size,
                   MapSizeCodeGenerator(), _1));
  total_number = map_size_generator(string_to_parse);
  std::cout << total_number << std::endl;*/
  
  std::vector<std::vector<std::vector<int>>> group;
  group_collection(total_number, group_size, group);
 // Generate map
   int num_of_grouped_file = 0;
   for (unsigned int i = 0; i != group.size(); i++) {
     std::map<std::string, std::string> state_to_group;
     generate_map(group[i], state_to_group);
     auto group_domino_code_generator = SinglePass<>(
         std::bind(&DominoToGroupDominoCodeGenerator::ast_visit_transform,
                   DominoToGroupDominoCodeGenerator(state_to_group), _1));
     std::string sketch_program =
     group_domino_code_generator(string_to_parse); 
     std::string filename =
         "/tmp/"+ src_filename + "_equivalent_" + std::to_string(num_of_grouped_file) + ".c";
     std::ofstream myfile;
     myfile.open(filename.c_str());
     myfile << sketch_program;
     myfile.close();
     num_of_grouped_file++;
   }
  return 0;
}
