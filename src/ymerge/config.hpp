#pragma once

#include <filesystem>
#include <map>
#include <optional>
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
  std::filesystem::path data_path;

  ymerge_repo(std::string name, std::string url, std::vector<std::string> allowed_signers);

 private:
  std::optional<std::map<std::string, std::string>> aur_whitelist = std::nullopt;

 public:
  std::map<std::string, std::string>& get_aur_whitelist();
};

std::vector<ymerge_repo>& get_repos();

}  // namespace ymerge::config
