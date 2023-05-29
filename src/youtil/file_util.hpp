#pragma once

#include <filesystem>
#include <optional>
#include <string>

/**
 * Convenience functions for basic file operations.
 */

namespace fly::file {

std::shared_ptr<std::string> read(std::filesystem::path);
void write(std::filesystem::path, const char *);
inline void write(std::filesystem::path p, std::string s) { write(p, s.c_str()); }
inline void touch(std::filesystem::path p) { write(p, ""); }

}
