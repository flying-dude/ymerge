#pragma once

/// Wrappers around libalpm

#include <alpm.h>

#include <optional>
#include <string>

namespace alpm {

struct pkg {
  alpm_pkg_t *pkg_;
  pkg(alpm_pkg_t *pkg_ = nullptr) : pkg_(pkg_) {}
  const char *pkgname() const { return alpm_pkg_get_name(pkg_); }
  const char *pkgver() const { return alpm_pkg_get_version(pkg_); };
  operator alpm_pkg_t *() const { return pkg_; }
};

/// a binary package database operated by pacman
struct db {
  alpm_db_t *db_;
  db(alpm_db_t *db_ = nullptr) : db_(db_) {}
  const char *get_name() { return alpm_db_get_name(db_); }
  std::optional<alpm::pkg> find_satisfier(std::string name);
  operator alpm_db_t *() const { return db_; }
};

/// main context handle that represents an instance of libalpm
struct handle {
  alpm_handle_t *alpm_;
  handle(const std::string &config_file);
  ~handle() { alpm_release(alpm_); }
  operator alpm_handle_t *() const { return alpm_; }

  alpm::db register_syncdb(const char *treename, int level) { return alpm_register_syncdb(alpm_, treename, level); }
  alpm::db get_localdb() { return alpm_get_localdb(alpm_); }
};

}  // namespace alpm
