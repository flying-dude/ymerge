#include <filesystem>
using namespace std::filesystem;

#include <fstream>
#include <iostream>
using namespace std;

#include "config.hpp"
#include "create_temporary_file.hpp"
#include "log.hpp"

namespace ymerge {

void sync() {
  for (auto& repo : config_::get_repos()) {
    path pkg = repo.data_path / "pkg";
    if (!is_directory(pkg)) { sudo("mkdir", "--parents", pkg); }

    path db = pkg / (repo.name + ".db.tar");
    if (!exists(db)) { sudo("repo-add", db); }

    path git_dir = repo.data_path / "git";
    path allowed_signers_file = repo.data_path / "allowed_signers";

    if (!exists(git_dir)) {
      sudo("mkdir", "--parents", git_dir.c_str());
      sudo("git", "clone", "--depth", "1", "--", repo.url, git_dir);
    } else {
      // https://stackoverflow.com/questions/2180270/check-if-current-directory-is-a-git-repository
      fly::cmd_options opt_rev_parse;
      opt_rev_parse.stdout_file = "/dev/null";

      exec_opt_throw(fmt::format("pkg dir does exist but is not a git repo: \"{}\"", git_dir.c_str()), opt_rev_parse,
                     "sudo", "git", "-C", git_dir.c_str(), "rev-parse", "--is-inside-work-tree");

      // https://stackoverflow.com/questions/41075972/how-to-update-a-git-shallow-clone
      sudo("git", "-C", git_dir.c_str(), "fetch", "--depth", "1");
      sudo("git", "-C", git_dir.c_str(), "reset", "--hard", "origin/main");
    }

    // create signers file, if it doesn't exist yet
    if (!exists(allowed_signers_file)) {
      auto tmpfile = fly::temporary_file::New();

      cout << "tmpfile: " << tmpfile << endl;
      std::ofstream output(tmpfile->path);
      for (auto& signer : repo.allowed_signers) output << signer << endl;
      output.close();

      sudo("cp", tmpfile->path, allowed_signers_file);
      sudo("git", "-C", git_dir.c_str(), "config", "gpg.ssh.allowedSignersFile", allowed_signers_file.c_str());
    }
  }

  // TODO sync binaries repos here as well, using built-in pacman facilities
  // sudo("pacman", "--sync", "--refresh");
}

}  // namespace ymerge
