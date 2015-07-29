#include <cassert>
#include "unique_var_generator.h"

std::string UniqueVarGenerator::get_unique_var() const {
  // Propose tmp_x, where x starts at var_suffix_ + 1
  // and keeps incrementing until at least some tmp_x
  // is unique and does not belong to var_set_
  var_suffix_ = var_suffix_ + 1;
  std::string candidate = "tmp" + std::to_string(var_suffix_);
  while (var_set_.find(candidate) != var_set_.end()) {
    var_suffix_++;
    candidate = "tmp" + std::to_string(var_suffix_);
  }
  assert(var_set_.find(candidate) == var_set_.end());
  var_set_.emplace(candidate);
  return candidate;
}
