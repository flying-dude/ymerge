#include "cfg.hpp"

#include <nlohmann/json.hpp>

#include "file_util.hpp"
#include "log.hpp"

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
static void init_config();

static vector<ymerge_repo> git_repos_;
vector<ymerge_repo>& git_repos() {
  init_config();
  return git_repos_;
}

static bool use_nspawn_ = false;
bool use_nspawn() {
  init_config();
  return use_nspawn_;
}

static void print_sample_config() { fmt::print("\n\n {} \n\n", default_config_ymerge); }

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
  if (!j.contains("repos")) {
    warn("/etc/ymerge.json -- entry 'repos' missing in json file");
    print_sample_config();
  } else if (!j.contains("repos") || j["repos"].empty()) {
    warn("/etc/ymerge.json -- no repositories specified. 'repos' object is empty.");
    print_sample_config();
  } else if (!j["repos"].is_object()) {
    warn("/etc/ymerge.json -- entry 'repos' has the wrong type. needs to be of type object, found: {}",
         j["repos"].type_name());
    print_sample_config();
  } else {
    for (auto const& repo : j["repos"].items()) {
      string name = repo.key();
      string url = repo.value()["url"];

      vector<string> allowed_signers;
      for (auto& signer : repo.value()["allowed-signers"]) allowed_signers.push_back(signer);

      git_repos_.push_back({name, url, allowed_signers});
    }
  }

  // check if use-nspawn flag is set
  if (j.contains("use-nspawn")) { use_nspawn_ = j["use-nspawn"]; }

  config_initialized = true;
}

}  // namespace ymerge::cfg
