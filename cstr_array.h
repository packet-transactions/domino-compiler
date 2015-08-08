#ifndef CSTR_ARRAY_H_
#define CSTR_ARRAY_H_

#include <cstring>

#include <exception>
#include <vector>
#include <map>

/// RAII class to encapsulate arrays of C strs
/// i.e. char ** and safely delete them when they
/// go out of scope. This is really a workaround
/// because some of Clang's methods take a char**
/// TODO: Consider using unique_ptrs.
class CstrArray {
 public:
  CstrArray(const std::vector<std::string> & t_strings)
      : strings_(t_strings),
        size_(static_cast<int>(strings_.size())) {
    // One const char* pointer for each string
    backing_store_ = new const char*[strings_.size()];

    // Point each string one by one
    for (uint32_t i = 0; i < strings_.size(); i++) {
      backing_store_[i] = strings_.at(i).c_str();
    }
  }

  ~CstrArray() noexcept {
    try {
      delete backing_store_;
    } catch (const std::exception & e) {}
  }

  /// Accessor
  const char ** get() const { return backing_store_; }

  /// Retrieve size
  int & size() { return size_; }

  /// Delete copy constructor, copy assignment, move constructor, move assignment
  CstrArray(const CstrArray &) = delete;
  CstrArray & operator=(const CstrArray &) = delete;
  CstrArray(const CstrArray &&) = delete;
  CstrArray & operator=(const CstrArray &&) = delete;

 private:
  const std::vector<std::string> strings_;
  int size_ = 0;
  const char ** backing_store_ = nullptr;
};

#endif  // CSTR_ARRAY_H_
