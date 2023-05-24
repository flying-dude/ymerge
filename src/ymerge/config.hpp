#pragma once

#include <filesystem>
#include <vector>

/**
 * Read ymerge configuration, initialized lazily when needed.
 * The configuration is globally available and read only once during program execution.
 * If no config file /etc/ymerge.yaml is present, a default configuration
 * will be used in that case (see config.cpp).
 */

namespace ymerge::config {

struct ymerge_repo {
  std::string name;
  std::string url;
  std::vector<std::string> allowed_signers;
  std::filesystem::path get_data_path() {
    return std::filesystem::path("/") / "var" / "lib" / "ymerge" / "repo" / name;
  }
};

std::vector<ymerge_repo>& get_repos();

// TODO delete this
inline ymerge_repo curated_aur_repo() { return get_repos()[0]; };

}  // namespace ymerge::config
