#pragma once

#include <vector>
#include <filesystem>

/**
 * Read ymerge configuration, initialized lazily when needed.
 * The configuration is globally available and read only once during program execution.
 * If no config file /etc/ymerge.yaml is present, a default configuration
 * will be used in that case (see config.cpp).
 */

namespace ymerge::config {

struct repo {
  std::string name;
  std::string url;
  std::string allowed_signers;
  std::filesystem::path get_path() { return std::filesystem::path("/") / "var" / "lib" / "ymerge" / "repo" / name; }
  void sync();
};

extern repo curated_aur_repo;

std::vector<repo>& get_repos();

}
