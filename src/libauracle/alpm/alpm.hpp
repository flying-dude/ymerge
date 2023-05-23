#pragma once

/// C++ Wrappers around libalpm

#include <alpm.h>

#include <optional>
#include <string>

namespace alpm {

struct list {
  alpm_list_t *list_;
  list(alpm_list_t *list_ = nullptr) : list_(list_) {}
  operator alpm_list_t *() const { return list_; }
  alpm_list_t operator->() { return *list_; }
};

struct pkg {
  alpm_pkg_t *pkg_;
  pkg(alpm_pkg_t *pkg_ = nullptr) : pkg_(pkg_) {}
  operator alpm_pkg_t *() const { return pkg_; }

  const char *pkgname() const { return alpm_pkg_get_name(pkg_); }
  const char *pkgver() const { return alpm_pkg_get_version(pkg_); };
};

struct group {
  alpm_group_t *group_;
  group(alpm_group_t *group_ = nullptr) : group_(group_) {}
  operator alpm_group_t *() const { return group_; }
};

/// a binary package database operated by pacman
struct db {
  alpm_db_t *db_;
  db(alpm_db_t *db_ = nullptr) : db_(db_) {}
  operator alpm_db_t *() const { return db_; }

  int unregister() { return alpm_db_unregister(db_); }
  const char *get_name() const { return alpm_db_get_name(db_); }
  int get_siglevel() { return alpm_db_get_siglevel(db_); }
  int get_valid() { return alpm_db_get_valid(db_); }

  /* server accessors */
  list get_servers() { return alpm_db_get_servers(db_); }
  int set_servers(alpm_list_t *servers) { return alpm_db_set_servers(db_, servers); }
  int add_server(const char *url) { return alpm_db_add_server(db_, url); }
  int remove_server(const char *url) { return alpm_db_remove_server(db_, url); }

  pkg get_pkg(alpm_db_t *db, const char *name) { return alpm_db_get_pkg(db_, name); }
  list get_pkgcache() const { return alpm_db_get_pkgcache(db_); }

  group get_group(const char *name) { return alpm_db_get_group(db_, name); }
  list get_groupcache() { return alpm_db_get_groupcache(db_); }

  int search(const alpm_list_t *needles, alpm_list_t **ret) { return alpm_db_search(db_, needles, ret); }

  /* usage accessors */
  int set_usage(int usage) { return alpm_db_set_usage(db_, usage); }
  int get_usage(int *usage) { return alpm_db_get_usage(db_, usage); }

  std::optional<alpm::pkg> find_satisfier(std::string name);
};

/// main context handle that represents an instance of libalpm
struct handle {
  alpm_handle_t *alpm_;
  handle(const char *config_file = "/etc/pacman.conf");
  virtual ~handle() { alpm_release(alpm_); }
  operator alpm_handle_t *() const { return alpm_; }

  alpm::db register_syncdb(const char *treename, int level) { return alpm_register_syncdb(alpm_, treename, level); }
  alpm::db get_localdb() { return alpm_get_localdb(alpm_); }

  int db_update(alpm_list_t *dbs, bool force) { return alpm_db_update(alpm_, dbs, force); }
};

}  // namespace alpm
