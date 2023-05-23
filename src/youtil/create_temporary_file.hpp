#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <random>
#include <string>

namespace fly {

#define TMP_PREFIX "cpp-tmp_"

/**
 * Create a temporary file in std::temp_directory_path() folder. Name is randomly generated
 * with given string prefix.
 */
std::filesystem::path create_temporary_file(std::string prefix = TMP_PREFIX, unsigned long long max_tries = 1000);

struct temporary_file {
  std::filesystem::path path;
  bool deleted = false;

  static std::shared_ptr<temporary_file> New(std::string prefix = TMP_PREFIX);
  void Delete();

  temporary_file(std::filesystem::path path) : path(path) {}
  ~temporary_file() { Delete(); }
};

}  // namespace fly
