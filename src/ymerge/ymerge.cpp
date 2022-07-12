#include <fmt/core.h>
#include <xdgdirs.h>

#include <cmd.hpp>
#include <file_contents.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <log.hpp>
#include <pkgbuild.hpp>
#include <sstream>
#include <unique_list.hpp>
#include <ymerge.hpp>

#include "auracle/auracle.hh"

/**
 * Package manager for the curated-aur. The command-line interface is inspired by Gentoo Portage:
 * https://dev.gentoo.org/~zmedico/portage/doc/man/emerge.1.html
 * https://wiki.gentoo.org/wiki/Full_manpages/emerge
 */

using namespace fmt;
using namespace std;
using namespace std::filesystem;
using namespace nlohmann;

namespace fly {

const char *help = R"(Usage: ymerge [options] [pkg...]
Version: )" YMERGE_VERSION R"(

An AUR Helper and Source-Based Package Manager for Arch Linux.

Options:
    --help -h      print help
    --version -v   print version

    --nocolor      turn off colors for produced shell output
    --noconfirm    skip confirmation and pick default answers automatically
    --quiet -q     minimize output
    --verbose      increase verbosity of output

    --remove -r    remove (uninstall) packages
    --srcinfo      only print srcinfo but do not build
    --sync -s      update local package database

Examples:
    Update the package database:
    $ ymerge --sync

    Update the the package manager:
    $ ymerge --sync ymerge

    Install the "xmake" and "build2" packages:
    $ ymerge xmake build2

    Remove the "xmake" and "build2" packages:
    $ ymerge --remove xmake build2)";

auto usage = R"(Usage: ymerge [options] [pkg...]
Help: ymerge --help
Version: )" YMERGE_VERSION;

namespace flag {
bool color = true;
bool confirm = true;
bool quiet = false;
bool remove = false;
bool srcinfo = false;
bool sync = false;
bool verbose = false;
bool version = false;
}  // namespace flag

path cache_dir;
path git_dir;
path pkg_dir;
path repo_dir;
json whitelist;

}  // namespace fly

using namespace auracle;
using namespace fly;

int main_throws(int argc, const char **argv);
int main(int argc, const char **argv) {
  try {
    return main_throws(argc, argv);
  } catch (const exception &err) {
    error("{}", err.what());
    return 1;
  }
}

/// this is the actual main-function. it can throw exceptions, which need to be caught by the calling function.
int main_throws(int argc, const char **argv) {
  if (argc == 1) {
    cout << usage << endl;
    return 0;
  }

  unique_list<string> pkgs;
  bool cli_error = false;
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      cout << help << endl;
      return 0;
    } else if (arg == "--confirm") {
      flag::confirm = true;
    } else if (arg == "--nocolor") {
      flag::color = false;
    } else if (arg == "--noconfirm") {
      flag::confirm = false;
    } else if (arg == "--quiet") {
      flag::quiet = true;
    } else if (arg == "--remove") {
      flag::remove = true;
    } else if (arg == "--srcinfo") {
      flag::srcinfo = true;
    } else if (arg == "--sync") {
      flag::sync = true;
    } else if (arg == "--verbose") {
      flag::verbose = true;
    } else if (arg == "--version") {
      flag::version = true;
    } else if (arg == "--") {
      warn("cli flag \"--\" currently has no function.");
    } else if (arg == "-" || arg.starts_with("--")) {
      error("unknown cli flag: \"{}\"", arg);
      cli_error = true;
    } else if (arg[0] == '-') {
      // facilitate multiple shorthandle flags like so: "ymerge -auq odin"
      auto arg_ = arg.c_str();
      int j = 1;
      for (char c = arg_[j]; c != '\0'; c = arg_[++j]) {
        switch (c) {
          case 'h': cout << help << endl; return 0;
          case 'q': flag::quiet = true; break;
          case 'r': flag::remove = true; break;
          case 's': flag::sync = true; break;
          case 'v': flag::version = true; break;
          default: error("unknown cli flag shorthandle: \"-{}\"", c); cli_error = true;
        }
      }
    } else {
      pkgs.insert(arg);
    }
  }

  // we don't exit on first error when parsing command line args but
  // report all errors that occured we -do- exit on error tho, which is
  // what we do here.
  if (cli_error) return 1;

  /* print version, if requested. intentionally not exiting here. user is allowed to request
   * further actions, like installing packages, in one command. */
  if (flag::version) { std::cout << "ymerge version: " YMERGE_VERSION << std::endl; }

  cache_dir = path(xdgCacheHome()) / "ymerge";
  git_dir = cache_dir / "curated-aur";
  pkg_dir = git_dir / "pkg";

  // TODO check if shell commands (like git) exist before using them
  // https://stackoverflow.com/questions/890894/portable-way-to-find-out-if-a-command-exists-c-c

  if (flag::sync) {
    if (auto err = ymerge::sync()) {
      error("{}", *err);
      return 1;
    }
  }

  // if no packages are requested for install, we are done at this point.
  if (pkgs.v.empty()) return 0;

  if (!exists(git_dir)) {
    fmt::print("Package dir \"{}\" not present.\n", pkg_dir.c_str());
    fmt::print("Use \"ymerge --sync\" to fetch package database.\n");

    cout << "Do you want me to perform \"ymerge --sync\" right now? [Y/n] ";

    string answer = "Y";
    if (flag::confirm) { cin >> answer; }

    if (answer.length() == 0) {
      error("Received empty answer.");
      return 1;
    }

    char c = tolower(answer.at(0));

    // exit program if they don't want to sync. should we return 0 or 1 here tho?
    if (c == 'n') return 0;

    if (c != 'y') {
      error("Could not understand answer \"{}\"", answer);
      return 1;
    }

    // c == 'y'
    if (auto err = ymerge::sync()) {
      error("{}", *err);
      return 1;
    }
  }

  auto whitelist_bytes = file_contents(git_dir / "aur-whitelist.json");
  if (auto err = !whitelist_bytes) {
    error("{}", *err);
    return 1;
  }
  whitelist = json::parse(*whitelist_bytes);

  auracle::Pacman pacman;

  // collect requested pkgbuilds. this could fail if user has specified a package that doesn't exist.
  vector<shared_ptr<pkgbuild>> recipes;
  bool missing_pkg_error = false;
  for (string &pkg : pkgs.v) {
    auto recipe = pkgbuild::New(pkg);
    if (auto err = !recipe) {
      error("{}", *err);
      missing_pkg_error = true;
    } else {
      recipes.push_back(*recipe);
    }
  }

  if (missing_pkg_error) return 1;

  for (auto &recipe : recipes) {
    if (auto err = recipe->merge()) {
      error("{}", *err);
      return 1;
    }
  }

  return 0;
}

namespace ymerge {

const char *curated_url = "https://github.com/flying-dude/curated-aur";

fly::xresult<void> sync(optional<path> git_dir_, optional<string> git_url_, optional<string> stdout,
                        optional<string> stderr) {
  path git_dir = git_dir_ ? *git_dir_ : fly::git_dir;
  string git_url = git_url_ ? *git_url_ : curated_url;

  cmd_options opt;
  opt.stdout_file = stdout;
  opt.stderr_file = stderr;

  if (!exists(git_dir)) {
    create_directories(git_dir);
    auto err = exec("git", "-C", cache_dir.c_str(), "clone", "--depth", "1", "--", ymerge::curated_url, git_dir);
    if (err) return err;
  } else {
    // https://stackoverflow.com/questions/2180270/check-if-current-directory-is-a-git-repository
    cmd_options opt_rev_parse;
    opt_rev_parse.stdout_file = "/dev/null";
    if (auto err = exec_opt(opt_rev_parse, "git", "-C", git_dir.c_str(), "rev-parse", "--is-inside-work-tree"))
      return fmt::format("pkg dir does exist but is not a git repo: \"{}\"", git_dir.c_str());

    // https://stackoverflow.com/questions/41075972/how-to-update-a-git-shallow-clone
    if (auto err = exec_opt(opt, "git", "-C", git_dir.c_str(), "fetch", "--depth", "1")) return err;

    if (auto err = exec_opt(opt, "git", "-C", git_dir.c_str(), "reset", "--hard", "origin/main")) return err;

    // is this even needed?
    // exec_opt(opt, "git", "-C", git_dir.c_str(), "clean", "-dfx");
  }

  return nullopt;
}

}  // namespace ymerge
