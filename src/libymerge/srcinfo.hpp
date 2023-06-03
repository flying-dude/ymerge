#include <optional>
#include <string>
#include <vector>

namespace ymerge {

struct optdepends_pair {
  std::string pkg_name;
  std::string desc;
};

/// Data parsed from .SRCINFO files.
struct srcinfo {
  std::string pkgbase;
  std::string pkgdesc;
  std::string pkgver;
  int pkgrel;
  std::string url;
  std::vector<std::string> arch;
  std::string license;
  std::vector<std::string> makedepends;
  std::vector<std::string> depends;
  std::vector<optdepends_pair> optdepends;
  std::vector<std::string> provides;
  std::vector<std::string> conflicts;
  std::vector<std::string> source;
  std::vector<std::string> sha256sums;
  std::vector<std::string> sha512sums;
  std::vector<std::string> b2sums;
  std::string pkgname;
  srcinfo(std::string);

  // string representation for printing
  std::string to_string();
};

}  // namespace ymerge
