#ifndef UNIQUE_VAR_GENERATOR_H_
#define UNIQUE_VAR_GENERATOR_H_

#include <set>
#include <string>

class UniqueVarGenerator {
 public:
  /// Constructor
  UniqueVarGenerator(const std::set<std::string> & initial_set) : var_set_(initial_set) {}

  /// Get unique variable
  std::string get_unique_var() const;

 private:
  /// Set of packet variables names,
  /// passed to the function in constructor
  mutable std::set<std::string> var_set_;

  /// Last unique variable suffix handed out,
  mutable int var_suffix_ = -1;
};

#endif  // UNIQUE_VAR_GENERATOR_H_
