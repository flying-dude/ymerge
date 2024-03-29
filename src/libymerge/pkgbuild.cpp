#include "pkgbuild.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "cfg.hpp"
#include "cmd.hpp"
#include "create_temporary_directory.hpp"
#include "file_util.hpp"
#include "log.hpp"
#include "ymerge.hpp"

using namespace std;
using namespace std::filesystem;
using namespace nlohmann;
using namespace fly;

namespace ymerge {

optional<shared_ptr<pkgbuild>> pkgbuild::New(string pkg_name) {
  // read PKGBUILD file from available git repos
  for (auto& r : cfg::git_repos()) {
    path recipe_dir = r.data_path / "git" / "pkg" / pkg_name;
    if (exists(recipe_dir)) {
      shared_ptr<pkgbuild> result = make_shared<pkgbuild_ymerge>(recipe_dir, r, pkg_name);
      return result;
    } else if (r.get_aur_whitelist().contains(pkg_name)) {
      string hash = r.get_aur_whitelist()[pkg_name];
      shared_ptr<pkgbuild> result = make_shared<pkgbuild_aur>(r, pkg_name, hash);
      return result;
    }
  }

  // no PKGBUILD found
  return nullopt;
}

void pkgbuild::remove_build_dir() {
  path build_dir = get_build_dir();
  info("Removing build dir: {}", build_dir.c_str());
  filesystem::remove_all(build_dir);
}

void pkgbuild::init_build_dir() {
  if (!init_build_dir_) {
    path build_dir = get_build_dir();

    // remove build dir in case it was not cleared up properly last time.
    if (filesystem::exists(build_dir)) remove_build_dir();

    info("Creating build dir: {}", build_dir.c_str());
    filesystem::create_directories(build_dir);

    init_build_dir(build_dir);

    // after full init we pass ownership to our build user "ymerge"
    exec("chown", "--recursive", "ymerge:ymerge", build_dir);

    init_build_dir_ = true;
  }
}

void pkgbuild::merge() {
  if (step::srcinfo() || step::install()) init_srcinfo();
  if (step::srcinfo()) print_srcinfo();
  if (step::install()) install();
}

void pkgbuild_aur::init_build_dir(std::filesystem::path& build_dir) {
  auto url = fmt::format("https://aur.archlinux.org/{}.git", working_name);
  exec("git", "clone", url, build_dir);
  git(build_dir, "reset", "--hard", git_hash);
}

void pkgbuild_ymerge::init_build_dir(std::filesystem::path& build_dir) {
  exec("cp", "--recursive", "--no-target-directory", pkg_folder, build_dir);
}

srcinfo& pkgbuild::init_srcinfo() {
  if (info_.has_value()) return info_.value();

  init_build_dir();

  path build_dir = get_build_dir();
  path file = build_dir / ".SRCINFO";
  if (!exists(file)) {
    // TODO implement "makepkg --printsrcinfo" inside ymerge
    cmd_options opt;
    opt.working_dir = build_dir;
    opt.stdout_file = file;
    // note: syntax "--user=ymerge" not supported by opendoas. "-u ymerge" works for both sudo and doas.
    exec_opt(opt, sudo_cmd(), "-u", "ymerge", "makepkg", "--printsrcinfo");
  }

  shared_ptr<string> sp = fly::file::read(file);
  string& info_source = *sp.get();
  info_ = info_source;
  return *info_;
}

void pkgbuild::print_srcinfo() { println("{}", info_->to_string().c_str()); }

// this will actually install the package using pacman
void pkgbuild::install() {
  init_build_dir();

  cmd_options opt;
  opt.working_dir = get_build_dir();
  // note: syntax "--user=ymerge" not supported by opendoas. "-u ymerge" works for both sudo and doas.
  exec_opt(opt, sudo_cmd(), "-u", "ymerge", "makepkg", "--syncdeps", "--noconfirm");

  string archive_name =
      (info_->pkgname + "-" + info_->pkgver + "-" + std::to_string(info_->pkgrel) + "-x86_64.pkg.tar.zst");

  path build_artifact_dir = repo.build_path();
  if (!std::filesystem::is_directory(build_artifact_dir)) filesystem::create_directories(build_artifact_dir);

  path archive_path = get_build_dir() / archive_name;
  info("move: {} -> {}", archive_name, build_artifact_dir.c_str());
  filesystem::rename(archive_path, build_artifact_dir / archive_name);
  exec("repo-add", repo.build_path() / "curated-aur.db.tar", build_artifact_dir / archive_name);

  if (flag::makepkg) return;

  exec("pacman", "--upgrade", "--noconfirm", build_artifact_dir / archive_name);
}

pkgbuild::~pkgbuild() {
  if (init_build_dir_) remove_build_dir();
}

}  // namespace ymerge
