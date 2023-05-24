#pragma once

#include <fmt/color.h>
#include <fmt/core.h>

#include <sstream>

#include "cmd.hpp"
#include "ymerge.hpp"

/** Logging functions based on fmtlib with optional colored output. */

namespace ymerge {

// "color specifiers for different parts of the message"
// https://github.com/fmtlib/fmt/issues/1100

// "Why is println missing?"
// https://github.com/fmtlib/fmt/issues/428
template <typename... T>
FMT_INLINE void println(fmt::format_string<T...> fmt, T &&...args) {
  const auto &vargs = fmt::make_format_args(args...);
  fmt::detail::is_utf8() ? vprint(fmt, vargs) : fmt::detail::vprint_mojibake(stdout, fmt, vargs);
  std::putc('\n', stdout);
}

template <typename... T>
FMT_INLINE void info(fmt::format_string<T...> fmt, T &&...args) {
  if (flag::quiet) return;

  fmt::print("[");
  if (flag::color)
    fmt::print(fg(fmt::color::yellow_green) | fmt::emphasis::bold, "info");
  else
    fmt::print("info");
  fmt::print("] ");

  const auto &vargs = fmt::make_format_args(args...);
  fmt::detail::is_utf8() ? vprint(fmt, vargs) : fmt::detail::vprint_mojibake(stdout, fmt, vargs);
  std::putc('\n', stdout);
}

template <typename... T>
FMT_INLINE void warn(fmt::format_string<T...> fmt, T &&...args) {
  if (flag::quiet) return;

  fmt::print(stderr, "[");
  if (flag::color)
    fmt::print(stderr, fg(fmt::color::pale_violet_red) | fmt::emphasis::bold, "warn");
  else
    fmt::print(stderr, "warn");
  fmt::print(stderr, "] ");

  const auto &vargs = fmt::make_format_args(args...);
  fmt::detail::is_utf8() ? vprint(stderr, fmt, vargs) : fmt::detail::vprint_mojibake(stderr, fmt, vargs);
  std::putc('\n', stderr);
}

template <typename... T>
FMT_INLINE void error(fmt::format_string<T...> fmt, T &&...args) {
  fmt::print(stderr, "[");
  if (flag::color)
    fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, "error");
  else
    fmt::print(stderr, "error");
  fmt::print(stderr, "] ");

  const auto &vargs = fmt::make_format_args(args...);
  fmt::detail::is_utf8() ? vprint(stderr, fmt, vargs) : fmt::detail::vprint_mojibake(stderr, fmt, vargs);
  std::putc('\n', stderr);
}

template <typename... T>
FMT_INLINE void todo(fmt::format_string<T...> fmt, T &&...args) {
  fmt::print("[");
  fmt::print(fg(fmt::color::purple) | fmt::emphasis::bold, "todo");
  fmt::print("] ");

  const auto &vargs = fmt::make_format_args(args...);
  fmt::detail::is_utf8() ? vprint(fmt, vargs) : fmt::detail::vprint_mojibake(stdout, fmt, vargs);
  std::putc('\n', stdout);
  throw std::runtime_error("");
}

// note: this should be reusable for info, warn, error etc. but so far i wasn't able to tame the template errors.
template <typename... T>
FMT_INLINE void log(const fmt::text_style &ts, std::string_view prefix, fmt::format_string<T...> fmt, T &&...args) {
  fmt::print("[");
  if (flag::color)
    fmt::print(ts, "{}", prefix);
  else
    fmt::print("{}", prefix);
  fmt::print("] ");

  const auto &vargs = fmt::make_format_args(args...);
  fmt::detail::is_utf8() ? vprint(fmt, vargs) : fmt::detail::vprint_mojibake(stdout, fmt, vargs);
  std::putc('\n', stdout);
}

template <typename... T>
FMT_INLINE void exec_print(fly::cmd_options &opt, fmt::color color, std::string prefix, T &&...args) {
  if (flag::quiet) {
    // suppress stdout in --quiet mode. do not override, if an stdout location was explicitly specified tho.
    if (!opt.stdout_file) opt.stdout_file = "/dev/null";
  } else {
    log(fmt::text_style(fg(color) | fmt::emphasis::bold), prefix, "{}", cmd2str(opt, args...).c_str());
  }
}

template <typename... T>
FMT_INLINE void exec_opt(fly::cmd_options opt, T &&...args) {
  exec_print(opt, fmt::color::deep_sky_blue, "exec", args...);
  cmd_opt(opt, args...);
}

/**
 *  execute command and throw the specified error message, if the command exits with non-zero
 *  exit code.
 */
template <typename... T>
FMT_INLINE void exec_opt_throw(std::string throw_msg, fly::cmd_options opt, T &&...args) {
  exec_print(opt, fmt::color::deep_sky_blue, "exec", args...);
  if (cmd_opt_return_value(opt, args...) != 0) throw std::runtime_error(throw_msg);
}

template <typename... T>
FMT_INLINE void exec(T &&...args) {
  exec_opt({}, args...);
}

template <typename... T>
FMT_INLINE void sudo_opt(fly::cmd_options opt, T &&...args) {
  exec_print(opt, fmt::color::maroon, "sudo", args...);
  cmd_opt(opt, "sudo", args...);
}

template <typename... T>
FMT_INLINE void sudo(T &&...args) {
  sudo_opt({}, args...);
}

}  // namespace ymerge
