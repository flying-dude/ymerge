#include "ymerge.hpp"

#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "auracle/auracle.hh"
#include "cmd.hpp"
#include "file_contents.hpp"
#include "log.hpp"
#include "pkgbuild.hpp"
#include "unique_list.hpp"
#include "xdgdirs.h"
#include "create_temporary_file.hpp"

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
    --quiet -q     minimize output
    --verbose      increase verbosity of output
    --yes -y       skip confirmation and pick default answers automatically

    --pretend -p   show what would be done and exit
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
bool pretend = false;
bool quiet = false;
bool remove = false;
bool srcinfo = false;
bool sync = false;
bool update = false;
bool verbose = false;
bool version = false;
}  // namespace flag

path curated_aur_dir = path("/") / "var" / "lib" / "ymerge" / "repo" / "curated-aur";
json whitelist;

}  // namespace fly

using namespace auracle;
using namespace fly;

bool ask(std::string question);
void add_recipe_to_list(vector<shared_ptr<pkgbuild>> &recipes, shared_ptr<pkgbuild> &recipe, auracle::Pacman &pacman,
                        bool &missing_pkg_error);

void main_throws(int argc, const char **argv);
int main(int argc, const char **argv) {
  try {
    main_throws(argc, argv);
  } catch (const exception &err) {
    string msg = err.what();
    if (msg.length() > 0) error("{}", err.what());
    return 1;
  }
}

/// this is the actual main-function. it can throw exceptions, which need to be caught by the calling function.
void main_throws(int argc, const char **argv) {
  if (argc == 1) {
    cout << usage << endl;
    return;
  }

  unique_list<string> pkgs;
  bool cli_error = false;
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      cout << help << endl;
      return;
    } else if (arg == "--ask") {
      flag::confirm = true;
    } else if (arg == "--nocolor") {
      flag::color = false;
    } else if (arg == "--pretend") {
      flag::pretend = true;
    } else if (arg == "--quiet") {
      flag::quiet = true;
    } else if (arg == "--remove") {
      flag::remove = true;
    } else if (arg == "--srcinfo") {
      flag::srcinfo = true;
    } else if (arg == "--sync") {
      flag::sync = true;
    } else if (arg == "--update") {
      flag::update = true;
    } else if (arg == "--verbose") {
      flag::verbose = true;
    } else if (arg == "--version") {
      flag::version = true;
    } else if (arg == "--yes") {
      flag::confirm = false;
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
          case 'h': cout << help << endl; return;
          case 'p': flag::pretend = true; break;
          case 'q': flag::quiet = true; break;
          case 'r': flag::remove = true; break;
          case 's': flag::sync = true; break;
          case 'u': flag::update = true; break;
          case 'v': flag::version = true; break;
          case 'y': flag::confirm = false; break;
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
  if (cli_error) throw std::invalid_argument("");

  /* print version, if requested. intentionally not exiting here. user is allowed to request
   * further actions, like installing packages, in one command. */
  if (flag::version) { std::cout << "ymerge version: " YMERGE_VERSION << std::endl; }

  if (flag::sync) ymerge::sync();

  // if no packages are requested for install, we are done at this point.
  if (pkgs.v.empty()) return;

  if (flag::remove) {
    sudo("pacman", "--remove", pkgs.v);
    return;
  }

  if (!exists(curated_aur_dir / "git")) {
    fmt::print("Package dir \"{}\" not present.\n", (curated_aur_dir / "git" / "pkg").c_str());
    fmt::print("Use \"ymerge --sync\" to fetch package database.\n");

    bool answer = ask("Do you want me to perform \"ymerge --sync\" right now?");

    // exit program if declined to perform sync. return error or not tho?
    if (answer == false) return;

    // answer == yes
    ymerge::sync();
  }

  // verify git commit before proceeding
  exec_opt_throw("could not verify git commit.",
    {}, "sudo", "git", "-C", (curated_aur_dir / "git").c_str(), "verify-commit", "HEAD");

  std::string whitelist_bytes = file_contents(curated_aur_dir / "git" / "aur-whitelist.json");
  whitelist = json::parse(whitelist_bytes);

  auracle::Pacman pacman;

  if (flag::update) {
    todo("implement --update flag");
    if (flag::confirm)
      sudo("pacman", "--sync", "--sysupgrade", "--sysupgrade");
    else
      sudo("pacman", "--noconfirm", "--sync", "--sysupgrade", "--sysupgrade");
  }

  // collect requested pkgbuilds. this could fail if user has specified a package that doesn't exist.
  vector<shared_ptr<pkgbuild>> recipes;
  bool missing_pkg_error = false;
  for (string &pkg : pkgs.v) {
    optional<shared_ptr<pkgbuild>> recipe = pkgbuild::New(pkg);
    if (!recipe) {
      error("package \"{}\" not found", pkg.c_str());
      missing_pkg_error = true;
    } else {
      add_recipe_to_list(recipes, *recipe, pacman, missing_pkg_error);
    }
  }

  if (missing_pkg_error) throw std::invalid_argument("");

  if (step::install()) {
    // TODO use shell colors to distinguish packages selected by the users from packages that are just dependencies
    std::cout << "About to compile and install the following package recipes:";
    for (auto it = recipes.begin(); it != recipes.end(); it++) {
      std::string full_name = it->get()->full_name();
      std::cout << " " << full_name;
    }
    std::cout << std::endl;

    bool answer = ask("Do you want to proceed?");

    // exit program if decided to abort. return error or not tho?
    if (answer == false) return;
  }

  // answer == yes
  for (shared_ptr<pkgbuild> &recipe : recipes) { recipe->merge(); }

  return;
}

bool ask(string question) {
  // shouldn't ask during non-interactive "--pretend"
  if (flag::pretend) return false;

  cout << question << " [Y/n] ";

  string answer = "Y";
  if (flag::confirm) { cin >> answer; }
  if (answer.length() == 0) throw runtime_error("Received empty answer.");

  char c = tolower(answer.at(0));
  if (c == 'y') return true;
  if (c == 'n') return false;

  throw runtime_error(fmt::format("Could not understand answer \"{}\"", answer));
}

/** recursively add a package recipe to the list of recipes, together with its dependencies. they are added in correct
 * order, so that dependencies are added first.
 */
void add_recipe_to_list(vector<shared_ptr<pkgbuild>> &recipes, shared_ptr<pkgbuild> &recipe, auracle::Pacman &pacman,
                        bool &missing_pkg_error) {
  /* check if recipe is already part of the list. this could happen in case of cyclic dependencies or if multiple
   * requested packages have common dependencies. warning about cycle detection could be appropriate here.
   */
  for (auto &r : recipes)
    if (r->working_name == recipe->working_name) return;

  fly::srcinfo &s = recipe->init_srcinfo();
  for (auto &pkg : s.makedepends) {
    if (pacman.HasPackage(pkg)) continue;

    optional<shared_ptr<pkgbuild>> recipe_makedepends = pkgbuild::New(pkg);
    if (!recipe_makedepends) {
      error("makedepends package \"{}\" not found", pkg.c_str());
      missing_pkg_error = true;
    } else {
      add_recipe_to_list(recipes, *recipe_makedepends, pacman, missing_pkg_error);
    }
  }

  for (auto &pkg : s.depends) {
    if (pacman.HasPackage(pkg)) continue;

    optional<shared_ptr<pkgbuild>> recipe_depends = pkgbuild::New(pkg);
    if (!recipe_depends) {
      error("depends package \"{}\" not found", pkg.c_str());
      missing_pkg_error = true;
    } else {
      add_recipe_to_list(recipes, *recipe_depends, pacman, missing_pkg_error);
    }
  }

  recipes.push_back(recipe);
}

namespace ymerge {

const char *curated_url = "https://github.com/flying-dude/curated-aur";
const char *allowed_signers = "dude@flyspace.dev ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIE9qJsZ35FLI61AYNgb9y+3ZgOBJpr9ebFv8jgkDymPT";

void sync() {
  sudo("pacman", "--sync", "--refresh");

  path git_dir = fly::curated_aur_dir / "git";
  path allowed_signers_file = fly::curated_aur_dir / "allowed_signers";

  if (!exists(git_dir)) {
    sudo("mkdir", "--parents", git_dir.c_str());
    //sudo("git", "clone", "--depth", "1", "--", ymerge::curated_url, git_dir);
  } else {
    // https://stackoverflow.com/questions/2180270/check-if-current-directory-is-a-git-repository
    cmd_options opt_rev_parse;
    opt_rev_parse.stdout_file = "/dev/null";

    exec_opt_throw(fmt::format("pkg dir does exist but is not a git repo: \"{}\"", git_dir.c_str()),
      opt_rev_parse, "sudo", "git", "-C", git_dir.c_str(), "rev-parse", "--is-inside-work-tree");

    // https://stackoverflow.com/questions/41075972/how-to-update-a-git-shallow-clone
    sudo("git", "-C", git_dir.c_str(), "fetch", "--depth", "1");
    sudo("git", "-C", git_dir.c_str(), "reset", "--hard", "origin/main");
  }

  if (!exists(allowed_signers_file)) {
    {
      auto tmpfile = temporary_file::New();

      cout << "tmpfile: " << tmpfile << endl;
      std::ofstream output(tmpfile->path);
      output << allowed_signers;
      output.close();

      sudo("cp", tmpfile->path, allowed_signers_file);
    }

    sudo("git", "-C", git_dir.c_str(), "config", "gpg.ssh.allowedSignersFile", allowed_signers_file.c_str());
  }
}

}  // namespace ymerge
