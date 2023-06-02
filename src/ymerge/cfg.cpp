#include "cfg.hpp"

#include <nlohmann/json.hpp>

#include "file_util.hpp"

using namespace std;
using namespace std::filesystem;
using json = nlohmann::json;

namespace ymerge::cfg {

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

static bool config_initialized = false;
static vector<ymerge_repo> repos;

// TODO read json config file in /etc/ymerge.json
static void init_config() {
  if (config_initialized) return;

  shared_ptr<string> config;
  if (exists("/etc/ymerge.json"))
    config = fly::file::read("/etc/ymerge.json");
  else
    config = make_shared<string>(default_config_ymerge);

  json j = json::parse(*config);

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

  config_initialized = true;
}

vector<ymerge_repo>& git_repos() {
  init_config();
  return repos;
}

}  // namespace ymerge::cfg
