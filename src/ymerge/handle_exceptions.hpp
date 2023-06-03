#pragma once

#include <exception>

namespace ymerge {

struct not_root_exception : std::exception {
  int result;
  not_root_exception(int result) : result(result) {}
};

}  // namespace ymerge
