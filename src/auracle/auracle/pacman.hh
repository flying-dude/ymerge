// SPDX-License-Identifier: MIT
#pragma once

#include <alpm.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace auracle {

struct Pacman {
  struct Package {
    Package(std::string pkgname, std::string pkgver) : pkgname(std::move(pkgname)), pkgver(std::move(pkgver)) {}
    std::string pkgname;
    std::string pkgver;
  };

  Pacman() : Pacman("/etc/pacman.conf") {}
  Pacman(const std::string &config_file);
  ~Pacman() { alpm_release(alpm_); }

  Pacman(const Pacman &) = delete;
  Pacman &operator=(const Pacman &) = delete;

  Pacman(Pacman &&) = default;
  Pacman &operator=(Pacman &&) = default;

  static int Vercmp(const std::string &a, const std::string &b);

  // Returns the name of the repo that the package belongs to, or std::nullopt
  // if the package was not found in any repo.
  std::optional<std::string> RepoForPackage(const std::string &package) const;
  bool HasPackage(const std::string &package) const { return RepoForPackage(package).has_value(); }

  bool DependencyIsSatisfied(const std::string &package) const;

  std::vector<Package> LocalPackages() const;
  std::optional<Package> GetLocalPackage(const std::string &name) const;

  alpm_handle_t *alpm_;
  alpm_db_t *local_db_;
};

}  // namespace auracle
