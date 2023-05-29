#pragma once

#include <filesystem>
#include <optional>
#include <string>

/**
 * Basic file operations as one-liners.
 */

namespace fly {
std::shared_ptr<std::string> read_file(std::filesystem::path);
void write_file(std::filesystem::path, const char *);
void write_file(std::filesystem::path, std::string);
}
