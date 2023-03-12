#include "repo.hpp"

using namespace std;

namespace ymerge {

repo curated_aur_repo = {"curated-aur" };

vector<repo> get_repos() {
  vector<repo> result;
  result.push_back( curated_aur_repo );
  return result;
}

}
