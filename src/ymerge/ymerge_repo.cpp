#include "ymerge_repo.hpp"

#include <nlohmann/json.hpp>

#include "file_util.hpp"

using namespace std;
using json = nlohmann::json;

namespace ymerge::cfg {

ymerge_repo::ymerge_repo(string name, string url, vector<string> allowed_signers)
    : name(name), url(url), allowed_signers(allowed_signers) {
  data_path = std::filesystem::path("/") / "var" / "cache" / "ymerge" / "repo" / name;
}

map<string, string>& ymerge_repo::get_aur_whitelist() {
  if (!aur_whitelist) {
    aur_whitelist = make_optional<map<string, string>>();

    shared_ptr<string> whitelist_bytes = fly::file::read(data_path / "git" / "aur-whitelist.json");
    json whitelist = json::parse(*whitelist_bytes);

    for (auto& item : whitelist.items()) aur_whitelist->emplace(item.key(), item.value());
  }

  return *aur_whitelist;
}

}  // namespace ymerge::cfg
