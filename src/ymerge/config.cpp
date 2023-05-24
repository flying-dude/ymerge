#include <optional>

#include "config.hpp"

using namespace std;

namespace ymerge::config {

repo curated_aur_repo = {
    "curated-aur",
    "https://github.com/flying-dude/curated-aur",
    "dude@flyspace.dev ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIE9qJsZ35FLI61AYNgb9y+3ZgOBJpr9ebFv8jgkDymPT"
};

static bool initialized = false;
static vector<repo> repos;

static void init_config() {
  if (initialized) return;

  repos.clear(); // shouldn't be necessary but just in case...
  repos.push_back(curated_aur_repo);

  initialized = true;
}

vector<repo>& get_repos() {
  init_config();
  return repos;
}

}
