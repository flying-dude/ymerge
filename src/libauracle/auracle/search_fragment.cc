// SPDX-License-Identifier: MIT
#include "search_fragment.hh"

#include <string.h>

#include <vector>

namespace auracle {

// We know that the AUR will reject search strings shorter than 2 characters.
constexpr std::string_view::size_type kMinCandidateSize = 2;

constexpr std::string_view regex_chars = R"(^.+*?$[](){}|\)";

std::string_view GetSearchFragment(std::string_view input) {
  std::vector<std::string_view> candidates;

  while (input.size() >= 2) {
    if (input.front() == '[' || input.front() == '{') {
      auto brace_end = input.find_first_of("]}");
      if (brace_end == input.npos) {
        // This may or may not be an invalid regex, e.g. we might have
        // "foo\[bar". In practice, this should never happen because package
        // names shouldn't have such characters.
        return std::string_view();
      }

      input.remove_prefix(brace_end);
      continue;
    }

    auto span = input.find_first_of(regex_chars);
    if (span == input.npos) {
      span = input.size();
    } else if (span == 0) {
      input.remove_prefix(1);
      continue;
    }

// clang-format off
/* flydude 07/2022

i got an assertion error at runtime from this trick, when compiling with "-Wp,-D_GLIBCXX_ASSERTIONS".

$ ./build/auracle search anything
/usr/include/c++/12.1.0/string_view:239: constexpr const std::basic_string_view<_CharT, _Traits>::value_type& std::basic_string_view<_CharT, _Traits>::operator[](size_type) const [with _CharT = char; _Traits = std::char_traits<char>; const_reference = const char&; size_type = long unsigned int]: Assertion '__pos < this->_M_len' failed.
fish: Job 1, './build/auracle search anything' terminated by signal SIGABRT (Abort)

these assertions can be disabled but the following cmake flag overrides it again: -DCMAKE_INSTALL_PREFIX:PATH='/usr'
guessing that the C++ standard lib is compiled with assertions?
==> solution: convert to c-string first and then obtain the character.

*/
// clang-format on

    // given 'cow?', we can't include w in the search
    // Looking one character past the end of the candidate is safe because we
    // know that our input is null delimited.
    std::string_view cand = input.substr(0, span);
    const char * cand_ = &cand[0];

    // char c = cand[span]; // assertion error due to string_view overflow (see above)
    char c = cand_[span];
    if (c == '?' || c == '*') {
      cand.remove_suffix(1);
    }

    if (cand.size() < kMinCandidateSize) {
      input.remove_prefix(1);
      continue;
    }

    candidates.push_back(cand);
    input.remove_prefix(span);
  }

  std::string_view longest;
  for (auto cand : candidates) {
    if (cand.size() > longest.size()) {
      longest = cand;
    }
  }

  return longest;
}

}  // namespace auracle
