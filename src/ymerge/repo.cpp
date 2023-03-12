#include "repo.hpp"

namespace ymerge {

std::vector<std::filesystem::path> get_repos() {
  std::vector<std::filesystem::path> result;
  result.push_back(std::filesystem::path("/") / "var" / "lib" / "ymerge" / "repo" / "curated-aur");
  return result;
}

}
