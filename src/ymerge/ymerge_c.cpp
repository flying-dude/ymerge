#include "ymerge_c.h"

#include <exception>
#include <string>

#include "log.hpp"
#include "sync.hpp"
#include "pkgbuild.hpp"

using namespace ymerge;
using namespace std;

int ymerge_sync() {
  try {
    ymerge::sync();
  } catch (const exception &err) {
    string msg = err.what();
    if (msg.length() > 0) error("{}", err.what());
    return 1;
  }
  return 0;
}

int ymerge_package_exists(const char * name) {
  return pkgbuild::New(name).has_value();
}
