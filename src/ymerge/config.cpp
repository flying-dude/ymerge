// config file parsing
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "config.hpp"
#include "file_contents.hpp"

using namespace std;
using namespace std::filesystem;

#include <iostream>

namespace ymerge::config {

const char* default_config = R"(
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
static vector<repo> repos;

repo curated_aur_repo;  // TODO delete this

// TODO read json config file in /etc/ymerge.yaml
static void init_config() {
  if (initialized) return;

  string config;
  if (exists("/etc/ymerge.json"))
    config = fly::file_contents("/etc/ymerge.json");
  else
    config = default_config;

  json j = json::parse(config);

  // obtain list of repositories
  {
    if (!j.contains("repos") || !j["repos"].is_object() || j["repos"].empty())
      throw runtime_error("/etc/ymerge.json -- no repositories specified");

    for (auto const& repo : j["repos"].items()) {
      string name = repo.key();
      string url = repo.value()["url"];
      string allowed_signers = repo.value()["allowed-signers"];
      repos.push_back({name, url, allowed_signers});
    }

    throw runtime_error("XXX");
    curated_aur_repo = repos[0];
  }

  initialized = true;
}

vector<repo>& get_repos() {
  init_config();
  return repos;
}

}  // namespace ymerge::config
