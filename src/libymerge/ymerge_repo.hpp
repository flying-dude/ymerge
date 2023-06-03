#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <vector>

namespace ymerge::cfg {

/**
 * Information about the git repositories listed in /etc/ymerge.json
 */
struct ymerge_repo {
  std::string name;
  std::string url;
  std::vector<std::string> allowed_signers;

  std::filesystem::path data_path;
  inline std::filesystem::path build_path() { return data_path / "build"; }

  ymerge_repo(std::string name, std::string url, std::vector<std::string> allowed_signers);

 private:
  std::optional<std::map<std::string, std::string>> aur_whitelist = std::nullopt;

 public:
  std::map<std::string, std::string>& get_aur_whitelist();
};

}  // namespace ymerge::cfg
