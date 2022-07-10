#include "alpm.hpp"

namespace alpm {

std::optional<alpm::pkg> db::find_satisfier(std::string name) {
  alpm_list_t *pkgcache = alpm_db_get_pkgcache(db_);
  alpm_pkg_t *pkg = alpm_find_satisfier(pkgcache, name.c_str());
  if (pkg == nullptr) { return std::nullopt; }
  return alpm::pkg{pkg};
}

}  // namespace alpm
