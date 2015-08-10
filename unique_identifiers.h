#ifndef UNIQUE_IDENTIFIERS_H_
#define UNIQUE_IDENTIFIERS_H_

#include <map>
#include <set>
#include <string>

class UniqueIdentifiers {
 public:
  /// Constructor
  UniqueIdentifiers(const std::set<std::string> & initial_set) : id_set_(initial_set) {}

  /// Get unique identifier
  std::string get_unique_identifier(const std::string & prefix = "tmp") const;

 private:
  /// Set of identifier names
  /// passed to the function in constructor
  mutable std::set<std::string> id_set_;

  /// Last unique id suffix handed out for every prefix
  mutable std::map<std::string, int>  id_suffix_ = {};
};

#endif  // UNIQUE_IDENTIFIERS_H_
