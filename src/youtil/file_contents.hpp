#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <xresult.hpp>

/// Read the contents of the given file into a string.
namespace fly {
xresult<std::string> file_contents(std::filesystem::path path);
}
