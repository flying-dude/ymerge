#pragma once

#include <vector>
#include <filesystem>

namespace ymerge {

struct repo;
std::vector<repo> get_repos();
extern repo curated_aur_repo;

struct repo {
  std::string name;

  std::string get_name() { return name; }
  std::filesystem::path get_path() { return std::filesystem::path("/") / "var" / "lib" / "ymerge" / "repo" / name; }
};

}