#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <fmt/core.h>
#include <xdgdirs.h>

#include "auracle/auracle.hh"

#include <cmd.hpp>
#include <file_contents.hpp>
#include <log.hpp>
#include <pkgbuild.hpp>
#include <unique_list.hpp>
#include <ymerge.hpp>

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

const char *usage = R"(Usage: ymerge [options] [pkg...]
Version: )" YMERGE_VERSION R"(

A Source-Based Package Manager for Arch Linux.

Options:
    --help -h             print help

    --nocolor      turn off colors for produced shell output
    --quiet -q     reduce output
    --remove -R    remove (uninstall) packages
    --srcinfo      only print srcinfo but do not build (unless --makepkg is also specified)
    --sync -s      update local package database
    --verbose      increase verbosity of output

Examples:
    Update the package database:
    $ ymerge --sync

    Update the the package manager:
    $ ymerge --sync ymerge

    Install the "xmake" and "build2" packages:
    $ ymerge xmake build2

    Remove the "xmake" and "build2" packages:
    $ ymerge --remove xmake build2)";

auto short_usage = R"(Usage: ymerge [options] [pkg...]
Help: ymerge --help
Version: )" YMERGE_VERSION;

#define CURATED_URL "https://github.com/flying-dude/curated-aur"

namespace flag {
	bool color = true;
	bool quiet = false;
	bool remove = false;
	bool srcinfo = false;
	bool sync = false;
	bool verbose = false;
} // namespace flag

path cache_dir;
path git_dir;
path pkg_dir;
path repo_dir;
json whitelist;

} // namespace fly

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

int main_throws(int argc, const char **argv) {
	if (argc == 1) {
		cout << short_usage << endl;
		return 0;
	}

	unique_list<string> pkgs;
	bool cli_error = false;
	for (int i = 1; i < argc; i++) {
		string arg = argv[i];

		if (arg == "-h" || arg == "--help") {
			cout << usage << endl;
			return 0;
		} else if (arg == "--nocolor") {
			flag::color = false;
		} else if (arg == "-q" || arg == "--quiet") {
			flag::quiet = true;
		} else if (arg == "-R" || arg == "--remove") {
			flag::remove = true;
		} else if (arg == "--srcinfo") {
			flag::srcinfo = true;
		} else if (arg == "--sync") {
			flag::sync = true;
		} else if (arg == "--verbose") {
			flag::verbose = true;
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
				case 'h': cout << usage << endl; return 0;
				case 'R': flag::remove = true; break;
				case 's': flag::sync = true; break;
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

	cache_dir = path(xdgCacheHome()) / "ymerge";
	repo_dir = cache_dir / "local-repo";
	git_dir = cache_dir / "curated-aur";
	pkg_dir = git_dir / "pkg";

	// TODO check if commands exist before using them
	// https://stackoverflow.com/questions/890894/portable-way-to-find-out-if-a-command-exists-c-c

	if (flag::sync) {
		if (!exists(cache_dir)) create_directories(cache_dir);

		if (!exists(git_dir)) {
			auto err = git("-C", cache_dir.c_str(), "clone", "--depth", "1", CURATED_URL);
			if (err) {
				error("{}", *err);
				return 1;
			}
		} else {
			// https://stackoverflow.com/questions/2180270/check-if-current-directory-is-a-git-repository
			cmd_options opt;
			opt.stdout_file = "/dev/null";
			if (auto err =
			        exec_opt(opt, "git", "-C", git_dir.c_str(), "rev-parse", "--is-inside-work-tree")) {
				error("pkg dir does exist but is not a git repo: \"{}\"", git_dir.c_str());
				return 1;
			}

			// https://stackoverflow.com/questions/41075972/how-to-update-a-git-shallow-clone
			if (auto err = git("-C", git_dir.c_str(), "fetch", "--depth", "1")) {
				error("{}", *err);
				return 1;
			}

			if (auto err = git("-C", git_dir.c_str(), "reset", "--hard", "origin/main")) {
				error("{}", *err);
				return 1;
			}

			// is this even needed?
			// git("-C", git_dir.c_str(), "clean", "-dfx");
		}
	}

	if (!exists(git_dir)) {
		error("package dir \"{}\" not present. use \"ymerge --sync\" to fetch package database.",
		      pkg_dir.c_str());
		return 1;
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
