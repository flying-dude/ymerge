#include <cmd.hpp>
#include <create_temporary_directory.hpp>
#include <file_contents.hpp>
#include <filesystem>
#include <fstream>
#include <log.hpp>
#include <nlohmann/json.hpp>
#include <pkgbuild.hpp>
#include <ymerge.hpp>

using namespace std;
using namespace std::filesystem;
using namespace nlohmann;

namespace fly {

xresult<shared_ptr<pkgbuild>> pkgbuild::New(string pkg) {
  if (pkg.ends_with("PKGBUILD")) {
    path PKGBUILD = absolute(path(pkg));
    if (!is_regular_file(PKGBUILD)) return fail(fmt::format("not a file: {}", PKGBUILD.c_str()));

    path recipe_dir = PKGBUILD.parent_path();
    shared_ptr<pkgbuild> result = make_shared<pkgbuild_raw>(recipe_dir, pkg);
    return result;
  }

  path recipe_dir = pkg_dir / pkg;
  if (exists(recipe_dir)) {
    shared_ptr<pkgbuild> result = make_shared<pkgbuild_raw>(recipe_dir, pkg);
    return result;
  } else if (whitelist.contains(pkg)) {
    string hash = whitelist[pkg];
    shared_ptr<pkgbuild> result = make_shared<pkgbuild_aur>(pkg, hash);
    return result;
  } else {
    return fail(fmt::format("package \"{}\" not found", pkg));
  }
}

optional<string> pkgbuild::merge() {
  auto tmp_ = temporary_directory::New(fmt::format("ymerge-{}_", working_name));
  if (auto err = !tmp_) return *err;
  build_dir = tmp_->path;
  info("build dir: {}", build_dir->c_str());

  if (step::srcinfo() || step::install()) {
    if (auto err = init_build_dir()) return *err;
    if (auto err = init_srcinfo()) return *err;
  }

  if (step::srcinfo()) print_srcinfo();

  if (step::install())
    if (auto err = install()) return *err;

  if (step::remove())
    if (auto err = remove()) return *err;

  return {};
}

xresult<void> pkgbuild_aur::init_build_dir() {
  auto url = fmt::format("https://aur.archlinux.org/{}.git", working_name);
  if (auto err = exec("git", "clone", url, *build_dir)) {
    return *err;
  } else {
    // https://stackoverflow.com/questions/3489173/how-to-clone-git-repository-with-specific-revision-changeset
    return exec("git", "-C", *build_dir, "reset", "--hard", git_hash);
  }
}

xresult<void> pkgbuild_raw::init_build_dir() {
  return exec("cp", "--recursive", "--no-target-directory", pkg_folder, *build_dir);
}

xresult<void> pkgbuild::init_srcinfo() {
  auto file = *build_dir / ".SRCINFO";
  if (!exists(file)) {
    cmd_options opt;
    opt.working_dir = *build_dir;
    opt.stdout_file = file;
    if (auto err = exec_opt(opt, "makepkg", "--printsrcinfo")) return *err;
  }

  auto data = file_contents(file);
  if (auto err = !data) return *err;
  info_ = *data;
  return {};
}

void pkgbuild::print_srcinfo() { println("{}", info_->to_string().c_str()); }

// this will actually install the package using pacman
xresult<void> pkgbuild::install() {
  cmd_options opt;
  opt.working_dir = *build_dir;
  if (flag::confirm)
    return exec_opt(opt, "makepkg", "--syncdeps", "--install");
  else
    return exec_opt(opt, "makepkg", "--syncdeps", "--install", "--noconfirm");
}

xresult<void> pkgbuild::remove() {
  // when we remove a package we won't even init srcinfo. that means we have to use working_name instead of
  // srcinfo->pkgname, since that one is N/A. probably these two are (always?) identical anyway.
  if (flag::confirm)
    return exec("sudo", "pacman", "--remove", working_name);
  else
    return exec("sudo", "pacman", "--noconfirm", "--remove", working_name);
}

}  // namespace fly
