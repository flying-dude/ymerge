#include <string>
#include <string_view>
#include <vector>

/**
 * Extra string functionality, that is not provided by the C++ standard library.
 */

namespace fly {

/// Split string by using the given delimiter as a separator.
std::vector<std::string> split(std::string_view str, std::string_view delim);

/// Strip leading and trailing whitespace from the string.
std::string strip(std::string_view inpt);

}  // namespace fly
