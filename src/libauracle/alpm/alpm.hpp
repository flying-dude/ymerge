#pragma once

/// Wrappers around libalpm

#include <alpm.h>

namespace alpm {

struct pkg {
  pkg(alpm_pkg_t *pkg_) : pkg_(pkg_) {}
  alpm_pkg_t *pkg_;
  const char *pkgname() const { return alpm_pkg_get_name(pkg_); }
  const char *pkgver() const { return alpm_pkg_get_version(pkg_); };
  operator alpm_pkg_t *() { return pkg_; }
};

}  // namespace alpm
