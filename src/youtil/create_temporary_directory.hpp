#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <random>
#include <string>

#include <xresult.hpp>
#include <ymerge.hpp>

namespace fly {

#define TMP_PREFIX "cpp-tmp_"

/**
 * Create a temporary directory in std::temp_directory_path() folder. Name is randomly generated
 * with given string prefix.
 */
xresult<std::filesystem::path> create_temporary_directory(std::string prefix = TMP_PREFIX,
                                                          unsigned long long max_tries = 1000);

/**
 * A type for temp dirs that will auto-create in constructor and auto-delete the dir in destructor.
 *
 * Use (static) temporary_directory::New() instead of constructor. This yields a shared pointer or
 * a possible error message.
 *
 * Use (non-static) temporary_directory::Delete() to invoke the destructor manually. This yields errors
 * in case something goes wrong during deletion. An internal boolean will keep track, if the destructor has already
 * been called. Therefore, calling ::Delete() multiple times is completely safe.
 */
struct temporary_directory {
	std::filesystem::path path;
	bool deleted = false;

	static xresult<std::shared_ptr<temporary_directory>> New(std::string prefix = TMP_PREFIX);
	xresult<void> Delete();

	temporary_directory(std::filesystem::path path) : path(path) {}
	~temporary_directory() { Delete(); }
};

} // namespace fly
