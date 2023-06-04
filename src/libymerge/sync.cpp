#include <filesystem>
using namespace std::filesystem;

#include <fstream>
#include <iostream>
using namespace std;

#include "cfg.hpp"
#include "create_temporary_file.hpp"
#include "log.hpp"

namespace ymerge {

void sync() {
  for (auto& repo : cfg::git_repos()) {
    path pkg = repo.build_path();
    if (!is_directory(pkg)) { filesystem::create_directories(pkg); }

    path db = pkg / (repo.name + ".db.tar");
    if (!exists(db)) { exec("repo-add", db); }

    path git_dir = repo.data_path / "git";
    path allowed_signers_file = repo.data_path / "allowed_signers";

    if (!exists(git_dir)) {
      filesystem::create_directories(git_dir);
      exec("git", "clone", "--depth", "1", "--", repo.url, git_dir);
    } else {
      try {
        // https://stackoverflow.com/questions/2180270/check-if-current-directory-is-a-git-repository
        // TODO https://libgit2.org/libgit2/#HEAD/group/revparse
        // https://www.reddit.com/r/AskProgramming/comments/7m6h2b/checking_if_a_folder_is_a_git_repository_using/
        // https://libgit2.org/libgit2/#v0.25.1/group/repository/git_repository_discover
        fly::cmd_options opt_rev_parse;
        opt_rev_parse.stdout_file = "/dev/null";
        opt_rev_parse.stderr_file = "/dev/null";
        opt_rev_parse.working_dir = git_dir;
        cmd_opt(opt_rev_parse, "git", "rev-parse", "--is-inside-work-tree");
      } catch (const std::runtime_error& err) {
        error("pkg dir exists but is not a git repo: \"{}\"", git_dir.c_str());
        throw err;
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

      filesystem::copy(tmpfile->path, allowed_signers_file);
      git(git_dir, "config", "gpg.ssh.allowedSignersFile", allowed_signers_file.c_str());
    }
  }

  // TODO sync binaries repos here as well, using built-in pacman facilities
  // exec("pacman", "--sync", "--refresh");
}

}  // namespace ymerge
