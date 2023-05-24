#include "config.hpp"

#include <fstream>
#include <iostream>

#include "create_temporary_file.hpp"
#include "log.hpp"

using namespace std;
using namespace std::filesystem;
using namespace fly;

namespace ymerge::config {

repo curated_aur_repo = {
    "curated-aur", "https://github.com/flying-dude/curated-aur",
    "dude@flyspace.dev ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIE9qJsZ35FLI61AYNgb9y+3ZgOBJpr9ebFv8jgkDymPT"};

static bool initialized = false;
static vector<repo> repos;

// TODO read yaml config file in /etc/ymerge.yaml
static void init_config() {
  if (initialized) return;

  repos.clear();  // shouldn't be necessary but just in case...
  repos.push_back(curated_aur_repo);

  initialized = true;
}

vector<repo>& get_repos() {
  init_config();
  return repos;
}

void repo::sync() {
  std::filesystem::path pkg = get_data_path() / "pkg";
  if (!is_directory(pkg)) { sudo("mkdir", "--parents", pkg); }

  std::filesystem::path db = pkg / (name + ".db.tar");
  if (!exists(db)) { sudo("repo-add", db); }

  path git_dir = get_data_path() / "git";
  path allowed_signers_file = get_data_path() / "allowed_signers";

  if (!exists(git_dir)) {
    sudo("mkdir", "--parents", git_dir.c_str());
    sudo("git", "clone", "--depth", "1", "--", url, git_dir);
  } else {
    // https://stackoverflow.com/questions/2180270/check-if-current-directory-is-a-git-repository
    cmd_options opt_rev_parse;
    opt_rev_parse.stdout_file = "/dev/null";

    exec_opt_throw(fmt::format("pkg dir does exist but is not a git repo: \"{}\"", git_dir.c_str()), opt_rev_parse,
                   "sudo", "git", "-C", git_dir.c_str(), "rev-parse", "--is-inside-work-tree");

    // https://stackoverflow.com/questions/41075972/how-to-update-a-git-shallow-clone
    sudo("git", "-C", git_dir.c_str(), "fetch", "--depth", "1");
    sudo("git", "-C", git_dir.c_str(), "reset", "--hard", "origin/main");
  }

  // verify the current git commit is signed by an authorized user
  if (!exists(allowed_signers_file)) {
    auto tmpfile = temporary_file::New();

    cout << "tmpfile: " << tmpfile << endl;
    std::ofstream output(tmpfile->path);
    output << allowed_signers;
    output.close();

    sudo("cp", tmpfile->path, allowed_signers_file);
    sudo("git", "-C", git_dir.c_str(), "config", "gpg.ssh.allowedSignersFile", allowed_signers_file.c_str());
  }
}

}  // namespace ymerge::config
