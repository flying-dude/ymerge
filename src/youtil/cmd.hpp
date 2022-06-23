#pragma once

#include <stdarg.h>

#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <xresult.hpp>

/**
 * This file facilitates executing shell commands with a convenient varargs pattern by simply
 * specifying the required arguments as a tuple of strings. Example:
 * > cmd("echo", "hello", "world");
 */

namespace fly {

/// Assign this one immediately, when entering main(argc, argv, envp)
/// https://stackoverflow.com/questions/31034993/how-does-a-program-inherit-environment-variables
extern char **envp;

/// Provide extra options, like working dir, to the executed command.
struct cmd_options {
	std::optional<std::string> stdout_file = std::nullopt;
	std::optional<std::string> stderr_file = std::nullopt;
	std::optional<std::filesystem::path> working_dir = std::nullopt;
};

/** Execute the given command using the given options. under the hood, this will pid=fork() and then waitpid(pid) for
 * the created child process. the child calls execvpe(argv) and then terminates. so this works like it would, if you
 * were running the command from a shell script (where the interpreter does the same thing for each given command). */
int exec_prog(std::vector<std::string> argv, cmd_options opt);

/// Helper function to fill a vector with given varargs.
template <typename T, typename Arg> void build_vector(std::vector<T> &argv, Arg arg) { argv.push_back(arg); }
template <typename T, typename Arg, typename... Args> void build_vector(std::vector<T> &argv, Arg arg, Args... args) {
	argv.push_back(arg);
	build_vector(argv, args...);
}

/// Create a string representation of the given command.
template <typename Arg> void cmd2str(std::stringstream &ss, Arg arg) { ss << " " << arg; }
template <typename Arg, typename... Argv> void cmd2str(std::stringstream &ss, Arg arg, Argv... argv) {
	ss << " " << arg;
	cmd2str(ss, argv...);
}

/// Create a string representation of the given command.
template <typename Arg, typename... Argv> std::string cmd2str(cmd_options opt, Arg arg, Argv... argv) {
	std::stringstream ss;
	if (opt.working_dir) ss << *opt.working_dir << " :: ";
	ss << arg; // first argument is not preceded by a space ' '.
	cmd2str(ss, argv...);
	return ss.str();
}

/// Execute a shell command with arguments and options.
template <typename... Argv> xresult<void> cmd_opt(cmd_options opt, Argv... argv) {
	std::vector<std::string> v;
	build_vector(v, argv...);
	if (exec_prog(v, opt) != 0)
		return std::string("command failed: ") + cmd2str(opt, argv...);
	else
		return {};
}

/// Execute a shell command with arguments.
template <typename... Argv> xresult<void> cmd(Argv... argv) { return cmd_opt({}, argv...); }

} // namespace fly
