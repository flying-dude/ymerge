#include <algorithm>
#include <cctype>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

/**
 * Extra string functionality, that is not provided by the C++ standard library.
 */

namespace fly {

/// Split string by using the given delimiter as a separator.
std::vector<std::string> split(std::string_view str, std::string_view delim);

// https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring

// trim from start (in place)
static inline void ltrim_inplace(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim_inplace(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim_inplace(std::string &s) {
  rtrim_inplace(s);
  ltrim_inplace(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
  ltrim_inplace(s);
  return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
  rtrim_inplace(s);
  return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
  trim_inplace(s);
  return s;
}

}  // namespace fly
