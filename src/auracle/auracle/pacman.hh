// SPDX-License-Identifier: MIT
#pragma once

#include <alpm.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace auracle {

struct SyncDB;

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
  std::optional<SyncDB> RepoForPackage(const std::string &package) const;

  // note: implementation cannot be put here directly, since SyncDB struct is forward-declared
  bool HasPackage(const std::string &package) const;

  bool DependencyIsSatisfied(const std::string &package) const;

  // local packages. note that local packages do not necessarily belong to a repo.
  // you can just create a simple PKGBUILD file and install a package from that.
  // no repo at all in that case.
  std::vector<Package> LocalPackages() const;
  std::optional<Package> GetLocalPackage(const std::string &name) const;

  alpm_handle_t *alpm_;
  alpm_db_t *local_db_;
};

// a binary package database operated by pacman
struct SyncDB {
  SyncDB(alpm_db_t *db_) : db(db_) {}
  alpm_db_t *db;
  std::string get_name() { return alpm_db_get_name(db); }
  std::optional<Pacman::Package> find_satisfier(std::string name);
};

}  // namespace auracle
