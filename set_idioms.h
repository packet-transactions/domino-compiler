#ifndef SET_IDIOMS_H_
#define SET_IDIOMS_H_

#include <ostream>
#include <algorithm>
#include <vector>
#include <set>

template <class T>
std::ostream & operator<<(std::ostream & out, const std::set<T> & set) {
  out << "{";
  for (const auto & node : set) {
    out << node << " ";
  }
  out << "}";
  return out;
}

// Define more idiomatic set union as '+' operator
template <class T>
std::set<T> operator+(const std::set<T> & a, const std::set<T> & b) {
  std::vector<T> temp;
  std::set_union(a.begin(), a.end(),
                 b.begin(), b.end(),                  
                 std::back_inserter(temp));
  return std::set<T>(temp.begin(), temp.end());
}

// Define more idiomatic set difference as '-' operator
template <class T>
std::set<T> operator-(const std::set<T> & a, const std::set<T> & b) {
  std::vector<T> temp;
  std::set_difference(a.begin(), a.end(),
                      b.begin(), b.end(),                  
                      std::back_inserter(temp));
  return std::set<T>(temp.begin(), temp.end());
}

// Define more idiomatic set intersection as '*' operator
template <class T>
std::set<T> operator*(const std::set<T> & a, const std::set<T> & b) {
  std::vector<T> temp;
  std::set_intersection(a.begin(), a.end(),
                        b.begin(), b.end(),                  
                        std::back_inserter(temp));
  return std::set<T>(temp.begin(), temp.end());
}

#endif // SET_IDIOMS_H_
