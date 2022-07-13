#pragma once

#include <filesystem>
#include <optional>
#include <string>

/// Read the contents of the given file into a string.
namespace fly {
std::string file_contents(std::filesystem::path path);
}
