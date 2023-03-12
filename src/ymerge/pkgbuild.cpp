#include "pkgbuild.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "cmd.hpp"
#include "create_temporary_directory.hpp"
#include "file_contents.hpp"
#include "log.hpp"
#include "ymerge.hpp"

using namespace std;
using namespace std::filesystem;
using namespace nlohmann;
using namespace fly;

namespace ymerge {

optional<shared_ptr<pkgbuild>> pkgbuild::New(string pkg_name) {
  if (pkg_name.ends_with("PKGBUILD")) {
    path PKGBUILD = absolute(path(pkg_name));
    if (!is_regular_file(PKGBUILD)) throw runtime_error(fmt::format("not a file: {}", PKGBUILD.c_str()));

    path recipe_dir = PKGBUILD.parent_path();
    shared_ptr<pkgbuild> result = make_shared<pkgbuild_raw>(recipe_dir, pkg_name);
    return result;
  }

  path recipe_dir = curated_aur_dir / "git" / "pkg" / pkg_name;
  if (exists(recipe_dir)) {
    shared_ptr<pkgbuild> result = make_shared<pkgbuild_raw>(recipe_dir, pkg_name);
    return result;
  } else if (whitelist.contains(pkg_name)) {
    string hash = whitelist[pkg_name];
    shared_ptr<pkgbuild> result = make_shared<pkgbuild_aur>(pkg_name, hash);
    return result;
  } else {
    return nullopt;
  }
}

path pkgbuild::init_build_dir() {
  if (build_dir_.has_value()) return build_dir_.value()->path;

  build_dir_ = temporary_directory::New(fmt::format("ymerge-{}_", working_name));
  init_build_dir(build_dir_.value()->path);
  return build_dir_.value()->path;
}

void pkgbuild::merge() {
  path build_dir = init_build_dir();
  info("build dir: {}", build_dir.c_str());

  if (step::srcinfo() || step::install()) init_srcinfo();
  if (step::srcinfo()) print_srcinfo();
  if (step::install()) install();
}

void pkgbuild_aur::init_build_dir(std::filesystem::path& build_dir) {
  auto url = fmt::format("https://aur.archlinux.org/{}.git", working_name);
  exec("git", "clone", url, build_dir);
  exec("git", "-C", build_dir, "reset", "--hard", git_hash);
}

void pkgbuild_raw::init_build_dir(std::filesystem::path& build_dir) {
  exec("cp", "--recursive", "--no-target-directory", pkg_folder, build_dir);
}

srcinfo& pkgbuild::init_srcinfo() {
  if (info_.has_value()) return info_.value();

  path build_dir = init_build_dir();
  path file = build_dir / ".SRCINFO";
  if (!exists(file)) {
    cmd_options opt;
    opt.working_dir = build_dir;
    opt.stdout_file = file;
    exec_opt(opt, "makepkg", "--printsrcinfo");
  }

  info_ = file_contents(file);
  return *info_;
}

void pkgbuild::print_srcinfo() { println("{}", info_->to_string().c_str()); }

// this will actually install the package using pacman
void pkgbuild::install() {
  cmd_options opt;
  opt.working_dir = init_build_dir();

  if (flag::confirm)
    exec_opt(opt, "makepkg", "--syncdeps");
  else
    exec_opt(opt, "makepkg", "--syncdeps", "--noconfirm");

  string archive_name = (info_->pkgname + "-" + info_->pkgver
                         + "-" + std::to_string(info_->pkgrel)
                         + "-x86_64.pkg.tar.zst");

  if(!std::filesystem::is_directory(custom_local_repo))
    sudo("mkdir", "--parents", custom_local_repo);

  sudo("mv", *opt.working_dir / archive_name , custom_local_repo);
  sudo("repo-add", custom_local_repo / "curated-aur.db.tar", custom_local_repo / archive_name);
  //sudo("pacman", "--sync", "--refresh");

  /*if (flag::confirm)
    sudo("pacman", "--upgrade", custom_local_repo / archive_name);
  else
    sudo("pacman", "--upgrade", "--noconfirm", custom_local_repo / archive_name);*/
}

void pkgbuild::remove() {
  // when we remove a package we won't even init srcinfo. that means we have to use working_name instead of
  // srcinfo->pkgname, since that one is N/A. probably these two are (always?) identical anyway.
  if (flag::confirm)
    sudo("pacman", "--remove", working_name);
  else
    sudo("pacman", "--noconfirm", "--remove", working_name);
}

}  // namespace ymerge
