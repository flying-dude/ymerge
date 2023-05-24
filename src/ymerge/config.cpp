// config file parsing
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "config.hpp"
#include "file_contents.hpp"

using namespace std;
using namespace std::filesystem;

#include <iostream>

namespace ymerge::config_ {

const char* default_config_ymerge = R"(
{
  "repos": {
    "curated-aur": {
      "url": "https://github.com/flying-dude/curated-aur",
      "allowed-signers": [
        "dude@flyspace.dev ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIE9qJsZ35FLI61AYNgb9y+3ZgOBJpr9ebFv8jgkDymPT"
      ]
    }
  }
}
)";

static bool initialized = false;
static vector<ymerge_repo> repos;

// TODO read json config file in /etc/ymerge.yaml
static void init_config() {
  if (initialized) return;

  string config;
  if (exists("/etc/ymerge.json"))
    config = fly::file_contents("/etc/ymerge.json");
  else
    config = default_config_ymerge;

  json j = json::parse(config);

  // obtain list of repositories
  {
    if (!j.contains("repos") || !j["repos"].is_object() || j["repos"].empty())
      throw runtime_error("/etc/ymerge.json -- no repositories specified");

    for (auto const& repo : j["repos"].items()) {
      string name = repo.key();
      string url = repo.value()["url"];

      vector<string> allowed_signers;
      for (auto& signer : repo.value()["allowed-signers"]) allowed_signers.push_back(signer);

      repos.push_back({name, url, allowed_signers});
    }
  }

  initialized = true;
}

ymerge_repo::ymerge_repo(string name, string url, vector<string> allowed_signers)
    : name(name), url(url), allowed_signers(allowed_signers) {
  data_path = std::filesystem::path("/") / "var" / "lib" / "ymerge" / "repo" / name;
}

map<string, string>& ymerge_repo::get_aur_whitelist() {
  if (!aur_whitelist) {
    aur_whitelist = make_optional<map<string, string>>();

    std::string whitelist_bytes = fly::file_contents(data_path / "git" / "aur-whitelist.json");
    json whitelist = json::parse(whitelist_bytes);

    for (auto& item : whitelist.items()) aur_whitelist->emplace(item.key(), item.value());
  }

  return *aur_whitelist;
}

vector<ymerge_repo>& get_repos() {
  init_config();
  return repos;
}

}  // namespace ymerge::config_
