#pragma once

#include <fmt/core.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "config.hpp"
#include "create_temporary_directory.hpp"
#include "srcinfo.hpp"

namespace ymerge {

/// A directory containing an Arch Linux PKGBUILD.
struct pkgbuild {
  // the ymerge repo from which this PKGBUILD comes
  config::ymerge_repo ymerge_repo;
  std::string working_name;  // tentative name used before reading .SRCINFO

  std::optional<std::shared_ptr<fly::temporary_directory>> build_dir_ = std::nullopt;

  pkgbuild(config::ymerge_repo ymerge_repo, std::string working_name)
      : ymerge_repo(ymerge_repo), working_name(working_name) {}

  virtual ~pkgbuild(){};
  static std::optional<std::shared_ptr<pkgbuild>> New(std::string);  // factory function for abstract type

  std::filesystem::path init_build_dir();
  virtual void init_build_dir(std::filesystem::path& build_dir) = 0;
  srcinfo& init_srcinfo();

  // build package or return error message if something went wrong
  void merge();

  // possible build steps
  void print_srcinfo();
  void install();
  void remove();

  // obtain full package name including pkgver and pkgrel.
  std::string full_name() {
    srcinfo& si = init_srcinfo();
    return fmt::format("{}-{}-{}", si.pkgname, si.pkgver, si.pkgrel);
  }

 private:
  std::optional<srcinfo> info_ = std::nullopt;
};

/// Obtain a PKGBUILD from Arch Linux AUR.
struct pkgbuild_aur : pkgbuild {
  std::string git_hash;
  pkgbuild_aur(config::ymerge_repo ymerge_repo, std::string working_name, std::string git_hash)
      : pkgbuild(ymerge_repo, working_name), git_hash(git_hash) {}
  ~pkgbuild_aur() {}
  void init_build_dir(std::filesystem::path& build_dir);
};

/// Specify a folder containing a PKGBUILD.
struct pkgbuild_ymerge : pkgbuild {
  std::filesystem::path pkg_folder;
  pkgbuild_ymerge(std::filesystem::path& pkg_folder, config::ymerge_repo ymerge_repo,
               std::string working_name = "PKGBUILD")
      : pkgbuild(ymerge_repo, working_name), pkg_folder(pkg_folder) {}
  ~pkgbuild_ymerge() {}
  void init_build_dir(std::filesystem::path& build_dir);
};

}  // namespace ymerge
