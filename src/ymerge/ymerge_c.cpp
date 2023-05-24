#include "ymerge_c.h"

#include <exception>
#include <set>
#include <string>
#include <vector>

#include "alpm.h"
#include "log.hpp"
#include "pkgbuild.hpp"
#include "sync.hpp"

extern "C" {
#include "pacman/conf.h"
#include "pacman/util.h"
}

using namespace ymerge;
using namespace std;

static vector<shared_ptr<pkgbuild>> queued_packages;

int ymerge_sync() {
  try {
    ymerge::sync();
  } catch (const exception& err) {
    string msg = err.what();
    if (msg.length() > 0) error("{}", err.what());
    return 0;
  }
  return 1;
}

static int ymerge_process_pkg(alpm_list_t* dbs, string pkg_name, set<string>& already_processed) {
  if (already_processed.contains(pkg_name))
    return 1;
  else
    already_processed.emplace(pkg_name);

  // see if pacman knows the current package
  alpm_pkg_t* pkg_t = alpm_find_dbs_satisfier(config->handle, dbs, pkg_name.c_str());

  // skip ignored packages
  if (alpm_errno(config->handle) == ALPM_ERR_PKG_IGNORED) {
    pm_printf(ALPM_LOG_WARNING, _("[ymerge] skipping target: %s\n"), pkg_name.c_str());
    return 1;  // returning the opposite of pacman here since 1 means no error, see process_targname() in sync.c for
               // comparison, where 0 would be no error.
  }

  // if pacman can provide it, we add it for pacman pkg list
  if (pkg_t) {
    int ret = alpm_add_pkg(config->handle, pkg_t);
    if (ret == -1) {
      alpm_errno_t err = alpm_errno(config->handle);
      pm_printf(ALPM_LOG_ERROR, "'%s': %s\n", alpm_pkg_get_name(pkg_t), alpm_strerror(err));
      return 0;  // once again opposite, 0 means error here
    }
  }

  // see if ymerge can provide the pkg
  optional<shared_ptr<pkgbuild>> pkgbuild_ = pkgbuild::New(pkg_name);
  if (!pkgbuild_.has_value()) return 0;

  pkgbuild& p = **pkgbuild_;
  srcinfo& si = p.get_srcinfo();

  // obtain depending packages
  std::vector<std::string> deps;
  deps.insert(deps.end(), si.makedepends.begin(), si.makedepends.end());
  deps.insert(deps.end(), si.depends.begin(), si.depends.end());

  bool error = false;
  for (string& dep : deps) error = error && ymerge_process_pkg(dbs, dep, already_processed);

  queued_packages.push_back(*pkgbuild_);
  return !error;
}

int ymerge_process_pkg(alpm_list_t* dbs, const char* pkg) {
  set<string> already_processed;
  return ymerge_process_pkg(dbs, pkg, already_processed);
}
