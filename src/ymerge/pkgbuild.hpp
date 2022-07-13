#pragma once

#include <fmt/core.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "create_temporary_directory.hpp"
#include "srcinfo.hpp"
#include "xresult.hpp"
#include "ymerge.hpp"

namespace fly {

/// A directory containing an Arch Linux PKGBUILD.
struct pkgbuild {
  std::optional<std::shared_ptr<temporary_directory>> build_dir_ = std::nullopt;
  std::string working_name;  // tentative name used before reading .SRCINFO

  pkgbuild(std::string working_name) : working_name(working_name) {}
  virtual ~pkgbuild(){};
  static xresult<std::shared_ptr<pkgbuild>> New(std::string);  // factory function for abstract type

  xresult<std::filesystem::path> init_build_dir();
  virtual xresult<void> init_build_dir(std::filesystem::path& build_dir) = 0;
  xresult<srcinfo*> init_srcinfo();

  std::filesystem::path package_archive() {
    return repo_dir / info_->pkgname / "-" / info_->pkgver / "-" / std::to_string(info_->pkgrel) /
           "-x86_64.pkg.tar.zst";
  }

  // build package or return error message if something went wrong
  std::optional<std::string> merge();

  // possible build steps
  void print_srcinfo();
  xresult<void> install();
  xresult<void> remove();

  // obtain full package name including pkgver and pkgrel.
  xresult<std::string> full_name() {
    auto si = init_srcinfo();
    if (auto err = !si) return fail(*err);
    srcinfo* s = si.success();
    return fmt::format("{}-{}-{}", s->pkgname, s->pkgver, s->pkgrel);
  }

 private:
  std::optional<srcinfo> info_ = std::nullopt;
};

/// Obtain a PKGBUILD from Arch Linux AUR.
struct pkgbuild_aur : pkgbuild {
  std::string git_hash;
  pkgbuild_aur(std::string working_name, std::string git_hash) : pkgbuild(working_name), git_hash(git_hash) {}
  ~pkgbuild_aur() {}
  xresult<void> init_build_dir(std::filesystem::path& build_dir);
};

/// Specify a folder containing a PKGBUILD.
struct pkgbuild_raw : pkgbuild {
  std::filesystem::path pkg_folder;
  pkgbuild_raw(std::filesystem::path& pkg_folder, std::string working_name = "PKGBUILD")
      : pkgbuild(working_name), pkg_folder(pkg_folder) {}
  ~pkgbuild_raw() {}
  xresult<void> init_build_dir(std::filesystem::path& build_dir);
};

}  // namespace fly
