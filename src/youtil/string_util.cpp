#include "string_util.hpp"

#include <sstream>

using namespace std;

namespace fly {

// https://stackoverflow.com/questions/13172158/c-split-string-by-line
vector<string> split(string_view str, string_view delim) {
  vector<string> strings;

  string::size_type pos = 0;
  string::size_type prev = 0;
  while ((pos = str.find(delim, prev)) != string::npos) {
    strings.push_back(string(str.substr(prev, pos - prev)));
    prev = pos + delim.length();
  }

  // To get the last substring (or only, if delimiter is not found)
  strings.push_back(string(str.substr(prev)));

  return strings;
}

}  // namespace fly
