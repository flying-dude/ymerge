#include <filesystem>

#include "config.hpp"

namespace ymerge {

void sync() {
  for (auto& repo : config::get_repos()) repo.sync();

  // TODO sync binaries repos here as well, using built-in pacman facilities
  // sudo("pacman", "--sync", "--refresh");
}

}  // namespace ymerge
