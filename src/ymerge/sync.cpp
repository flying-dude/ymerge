#include <filesystem>
#include <fstream>
#include <iostream>

#include "config.hpp"
#include "create_temporary_file.hpp"
#include "log.hpp"

using namespace std;
using namespace std::filesystem;
using namespace fly;
using namespace ymerge::config;

namespace ymerge {

void sync() {
  // need to make sure custom local repo db exists before running pacman --sync.
  for (repo& repo : get_repos()) {
    path pkg = repo.get_path() / "pkg";
    path db = pkg / (repo.name + ".db.tar");

    if (!is_directory(pkg)) { sudo("mkdir", "--parents", pkg); }

    if (!exists(db)) { sudo("repo-add", db); }
  }

  sudo("pacman", "--sync", "--refresh");

  for (auto& repo_ : config::get_repos()) {
    path git_dir = repo_.get_path() / "git";
    path allowed_signers_file = repo_.get_path() / "allowed_signers";

    if (!exists(git_dir)) {
      sudo("mkdir", "--parents", git_dir.c_str());
      sudo("git", "clone", "--depth", "1", "--", repo_.url, git_dir);
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
      output << repo_.allowed_signers;
      output.close();

      sudo("cp", tmpfile->path, allowed_signers_file);
      sudo("git", "-C", git_dir.c_str(), "config", "gpg.ssh.allowedSignersFile", allowed_signers_file.c_str());
    }
  }
}

}  // namespace ymerge
