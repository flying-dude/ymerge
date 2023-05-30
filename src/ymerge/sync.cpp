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
      opt_rev_parse.working_dir = git_dir;

      try {
        exec_opt(opt_rev_parse, "git", "rev-parse", "--is-inside-work-tree");
      } catch (const std::runtime_error& err) {
        throw std::runtime_error(fmt::format("pkg dir exists but is not a git repo: \"{}\"", git_dir.c_str()));
      }

      // https://stackoverflow.com/questions/41075972/how-to-update-a-git-shallow-clone
      git(git_dir, "fetch", "--depth", "1");
      git(git_dir, "reset", "--hard", "origin/main");
    }

    // create signers file, if it doesn't exist yet
    if (!exists(allowed_signers_file)) {
      auto tmpfile = fly::temporary_file::New();

      cout << "tmpfile: " << tmpfile << endl;
      std::ofstream output(tmpfile->path);
      for (auto& signer : repo.allowed_signers) output << signer << endl;
      output.close();

      sudo("cp", tmpfile->path, allowed_signers_file);
      git(git_dir, "config", "gpg.ssh.allowedSignersFile", allowed_signers_file.c_str());
    }
  }

  // TODO sync binaries repos here as well, using built-in pacman facilities
  // sudo("pacman", "--sync", "--refresh");
}

}  // namespace ymerge
