#pragma once

#include <stdarg.h>

#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

/**
 * This file facilitates executing shell commands with a convenient varargs pattern by simply
 * specifying the required arguments as a tuple of strings. Example:
 * > cmd("echo", "hello", "world");
 */

namespace fly {

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

/// Create a string representation of the given command.
inline void cmd2str(std::stringstream &ss) {}  // required for case of empty argument array
template <typename Args>
inline void cmd2str(std::stringstream &ss, std::string prefix, std::vector<Args> args) {
  bool first = true;
  for (auto &arg : args) {
    ss << (first ? prefix : " ") << arg;
    first = false;
  }
}
template <typename Arg>
inline void cmd2str(std::stringstream &ss, std::string prefix, Arg arg) {
  ss << prefix << arg;
}
template <typename Arg, typename... Argv>
inline void cmd2str(std::stringstream &ss, std::string prefix, Arg arg, Argv... argv) {
  cmd2str(ss, prefix, arg);
  cmd2str(ss, " ", argv...);
}

/// Create a string representation of the given command.
template <typename Arg, typename... Argv>
inline std::string cmd2str(cmd_options opt, Arg arg, Argv... argv) {
  std::stringstream ss;

  std::string prefix = "";
  if (opt.working_dir) prefix = opt.working_dir->string() + " :: ";

  cmd2str(ss, prefix, arg, argv...);
  return ss.str();
}

/// Helper function to fill a vector with given varargs.
template <typename T, typename Args>
inline void build_vector(std::vector<T> &argv, std::vector<Args> args) {
  for (auto &arg : args) argv.push_back(arg);
}
template <typename T, typename Arg>
inline void build_vector(std::vector<T> &argv, Arg arg) {
  argv.push_back(arg);
}
template <typename T, typename Arg, typename... Args>
inline void build_vector(std::vector<T> &argv, Arg arg, Args... args) {
  build_vector(argv, arg);
  build_vector(argv, args...);
}

/// Execute a shell command with arguments and options.
template <typename... Argv>
inline void cmd_opt(cmd_options opt, Argv... argv) {
  std::vector<std::string> v;
  build_vector(v, argv...);
  if (exec_prog(v, opt) != 0) throw std::runtime_error("Command failed: " + cmd2str(opt, argv...));
}

template <typename... Argv>
inline bool cmd_opt_return_value(cmd_options opt, Argv... argv) {
  std::vector<std::string> v;
  build_vector(v, argv...);
  return exec_prog(v, opt);
}

/// Execute a shell command with arguments.
template <typename... Argv>
inline void cmd(Argv... argv) {
  cmd_opt({}, argv...);
}

}  // namespace fly
