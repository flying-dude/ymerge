#pragma once

#include <algorithm>
#include <vector>

namespace fly {

/**
 * A list type that has unique entries. Will no-op when trying to insert an already existing element.
 * Special case of a set that retains insertion order. Use field unique_list::v to access the actual list.
 */
template <typename T, typename Alloc = std::allocator<T>>
struct unique_list {
  std::vector<T, Alloc> v;

  // https://stackoverflow.com/questions/24139428/check-if-element-is-in-the-list-contains#comment63040680_24139474
  bool contains(T &t) {
    bool found = (std::find(v.begin(), v.end(), t) != v.end());
    return found;
  }

  /**
   * Insert new element, if it does not already exist. Returns true, if it was inserted. And returns false if it
   * was not inserted because it was already part of the list.
   */
  bool insert(T &t) {
    if (!contains(t)) {
      v.push_back(std::forward<T>(t));
      return true;
    } else {
      return false;
    }
  }
};

}  // namespace fly
