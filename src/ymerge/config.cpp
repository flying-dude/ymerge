#include "config.hpp"

using namespace std;

namespace ymerge::config {

repo curated_aur_repo = {
    "curated-aur",
    "https://github.com/flying-dude/curated-aur",
    "dude@flyspace.dev ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIE9qJsZ35FLI61AYNgb9y+3ZgOBJpr9ebFv8jgkDymPT"
};

vector<repo> get_repos() {
  vector<repo> result;
  result.push_back( curated_aur_repo );
  return result;
}

}
