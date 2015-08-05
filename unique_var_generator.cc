#include <cassert>
#include "unique_var_generator.h"

std::string UniqueVarGenerator::get_unique_var(const std::string & prefix) const {
  // Propose prefix_x,
  // where x starts at var_suffix_.at(prefix) + 1
  // and keeps incrementing until at least some prefix_x
  // is unique and does not belong to var_set_
  if (var_suffix_.find(prefix) == var_suffix_.end()) {
    var_suffix_[prefix] = -1;
  }
  var_suffix_.at(prefix)++;
  std::string candidate = prefix + std::to_string(var_suffix_.at(prefix));
  while (var_set_.find(candidate) != var_set_.end()) {
    var_suffix_.at(prefix)++;
    candidate = prefix + std::to_string(var_suffix_.at(prefix));
  }
  assert(var_set_.find(candidate) == var_set_.end());
  var_set_.emplace(candidate);
  return candidate;
}
