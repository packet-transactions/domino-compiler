#include "unique_identifiers.h"

#include <cassert>

std::string UniqueIdentifiers::get_unique_identifier(const std::string & prefix) const {
  // Propose prefix_x,
  // where x starts at id_suffix_.at(prefix) + 1
  // and keeps incrementing until at least some prefix_x
  // is unique and does not belong to id_set_
  if (id_suffix_.find(prefix) == id_suffix_.end()) {
    id_suffix_[prefix] = -1;
  }
  id_suffix_.at(prefix)++;
  std::string candidate = prefix + std::to_string(id_suffix_.at(prefix));
  while (id_set_.find(candidate) != id_set_.end()) {
    id_suffix_.at(prefix)++;
    candidate = prefix + std::to_string(id_suffix_.at(prefix));
  }
  assert(id_set_.find(candidate) == id_set_.end());
  id_set_.emplace(candidate);
  return candidate;
}
