// SPDX-License-Identifier: MIT
#pragma once

#include <alpm.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "alpm/alpm.hpp"

namespace auracle {

struct Pacman {
  alpm::handle alpm_;
  alpm::db local_db_;

  Pacman(const char *config_file = "/etc/pacman.conf");

  Pacman(const Pacman &) = delete;
  Pacman &operator=(const Pacman &) = delete;

  Pacman(Pacman &&) = default;
  Pacman &operator=(Pacman &&) = default;

  static int Vercmp(const std::string &a, const std::string &b);

  // Returns the name of the repo that the package belongs to, or std::nullopt
  // if the package was not found in any repo.
  std::optional<alpm::db> RepoForPackage(const std::string &package) const;

  bool HasPackage(const std::string &package) const { return RepoForPackage(package).has_value(); }

  bool DependencyIsSatisfied(const std::string &package) const;

  // local packages. note that local packages do not necessarily belong to a repo.
  // you can just create a simple PKGBUILD file and install a package from that.
  // no repo at all in that case.
  std::vector<alpm::pkg> LocalPackages() const;
  std::optional<alpm::pkg> GetLocalPackage(const std::string &name) const;
};

}  // namespace auracle
