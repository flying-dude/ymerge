#pragma once

#include <filesystem>
#include <optional>
#include <string>

/**
 * Convenience functions for basic file operations.
 */

namespace fly {

std::shared_ptr<std::string> read_file(std::filesystem::path);
void write_file(std::filesystem::path, const char *);
inline void write_file(std::filesystem::path p, std::string s) { write_file(p, s.c_str()); }

}
