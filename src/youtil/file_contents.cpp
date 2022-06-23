#include <fstream>

#include <fmt/core.h>

#include "file_contents.hpp"

using namespace std;
using namespace std::filesystem;

namespace fly {

// https://blog.insane.engineer/post/cpp_read_file_into_string/
/** Returns a std::string, which represents the raw bytes of the file. */
xresult<std::string> file_contents(std::filesystem::path path) {
	// Sanity check
	if (!std::filesystem::is_regular_file(path)) return fail(fmt::format("not a file: {}", path.c_str()));

	// Open the file
	// Note that we have to use binary mode as we want to return a string
	// representing matching the bytes of the file on the file system.
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.is_open()) return fail(fmt::format("could not open file: {}", path.c_str()));

	// Read contents
	const std::size_t &size = std::filesystem::file_size(path);
	std::string content(size, '\0');
	file.read(content.data(), size);

	// Close the file
	file.close();

	return content;
}

} // namespace fly
