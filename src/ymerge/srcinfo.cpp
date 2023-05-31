#include <srcinfo.hpp>
#include <sstream>
#include <string_util.hpp>

using namespace std;
using namespace fly;

namespace ymerge {

srcinfo::srcinfo(string info) {
  auto lines = split(info, "\n");
  for (string line : lines) {
    line = trim_copy(line);
    if (line.empty()) continue;
    auto pair = split(line, " = ");

    string &key = pair[0];
    string &value = pair[1];

    if (key == "pkgbase")
      pkgbase = value;
    else if (key == "pkgdesc")
      pkgdesc = value;
    else if (key == "pkgver")
      pkgver = value;
    else if (key == "pkgrel")
      pkgrel = stoi(value);
    else if (key == "url")
      url = value;
    else if (key == "arch")
      arch.push_back(value);
    else if (key == "license")
      license = value;
    else if (key == "makedepends")
      makedepends.push_back(value);
    else if (key == "depends")
      depends.push_back(value);
    else if (key == "optdepends") {
      auto split_ = split(value, ": ");
      optdepends.push_back({split_[0], split_[1]});
    } else if (key == "provides")
      provides.push_back(value);
    else if (key == "conflicts")
      conflicts.push_back(value);
    else if (key == "source")
      source.push_back(value);
    else if (key == "sha256sums")
      sha256sums.push_back(value);
    else if (key == "sha512sums")
      sha512sums.push_back(value);
    else if (key == "b2sums")
      b2sums.push_back(value);
    else if (key == "pkgname")
      pkgname = value;
  }
}

string srcinfo::to_string() {
  stringstream ss;
  ss << "pkgbase :: " << pkgbase << endl;
  ss << "pkgdesc :: " << pkgdesc << endl;
  ss << "pkgver :: " << pkgver << endl;
  ss << "pkgrel :: " << pkgrel << endl;
  ss << "url :: " << url << endl;

  ss << "arch ::";
  for (auto &a : arch) ss << " " << a;
  ss << endl;

  ss << "license :: " << license << endl;

  ss << "makedepends ::";
  for (auto &a : makedepends) ss << " " << a;
  ss << endl;

  ss << "depends ::";
  for (auto &a : depends) ss << " " << a;
  ss << endl;

  ss << "optdepends:" << endl;
  for (auto &a : optdepends) ss << "    " << a.pkg_name << " :: " << a.desc << endl;

  ss << "provides ::";
  for (auto &a : provides) ss << " " << a;
  ss << endl;

  ss << "conflicts ::";
  for (auto &a : conflicts) ss << " " << a;
  ss << endl;

  ss << "source:" << endl;
  for (auto &a : source) ss << "    " << a << endl;

  ss << "sha256sums:" << endl;
  for (auto &a : sha256sums) ss << "    " << a << endl;

  ss << "sha512sums:" << endl;
  for (auto &a : sha512sums) ss << "    " << a << endl;

  ss << "b2sums:" << endl;
  for (auto &a : b2sums) ss << "    " << a << endl;

  ss << "pkgname :: " << pkgname;
  return ss.str();
}

}  // namespace ymerge
