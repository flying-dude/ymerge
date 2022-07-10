// SPDX-License-Identifier: MIT
#include "pacman.hh"

#include <fmt/core.h>

#include <string>
#include <string_view>
#include <vector>

namespace auracle {

Pacman::Pacman(const char *config_file) : alpm_(config_file) { this->local_db_ = alpm_.get_localdb(); }

std::optional<alpm::db> Pacman::RepoForPackage(const std::string &package) const {
  for (auto i = alpm_get_syncdbs(alpm_); i != nullptr; i = i->next) {
    auto db = static_cast<alpm_db_t *>(i->data);
    auto pkgcache = alpm_db_get_pkgcache(db);

    if (alpm_find_satisfier(pkgcache, package.c_str()) != nullptr) { return db; }
  }

  return std::nullopt;
}

bool Pacman::DependencyIsSatisfied(const std::string &package) const {
  auto *cache = alpm_db_get_pkgcache(local_db_);
  return alpm_find_satisfier(cache, package.c_str()) != nullptr;
}

std::optional<alpm::pkg> Pacman::GetLocalPackage(const std::string &name) const {
  alpm_pkg_t *pkg = alpm_db_get_pkg(local_db_, name.c_str());
  if (pkg == nullptr) { return std::nullopt; }

  return alpm::pkg{pkg};
}

std::vector<alpm::pkg> Pacman::LocalPackages() const {
  std::vector<alpm::pkg> packages;

  for (auto i = alpm_db_get_pkgcache(local_db_); i != nullptr; i = i->next) {
    const auto pkg = static_cast<alpm_pkg_t *>(i->data);
    packages.emplace_back(pkg);
  }

  return packages;
}

// static
int Pacman::Vercmp(const std::string &a, const std::string &b) { return alpm_pkg_vercmp(a.c_str(), b.c_str()); }

}  // namespace auracle