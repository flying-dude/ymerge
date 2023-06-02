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

namespace ymerge::config_ {

/**
 * Information about the git repositories listed in /etc/ymerge.json
 */
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

/// returns the git repositories listed in /etc/ymerge.json
std::vector<ymerge_repo>& git_repos();

}  // namespace ymerge::config_
