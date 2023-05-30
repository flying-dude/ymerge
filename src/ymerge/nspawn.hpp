#include "log.hpp"

namespace ymerge {

extern std::filesystem::path nspawn_dir;

extern void init_nspawn();

template <typename... T>
FMT_INLINE void nspawn_opt(fly::cmd_options opt, T &&...args) {
  // check if systemd is actually running, before invoking systemd-nspawn
  // we might be running inside a container or chroot, where systemd isn't started
  // https://superuser.com/questions/1017959/how-to-know-if-i-am-using-systemd-on-linux/1631444#1631444
  std::filesystem::path sd_booted = std::filesystem::path("/") / "run" / "systemd" / "system";
  if (!std::filesystem::is_directory(sd_booted))
    throw std::runtime_error(R"(Cannot initialize build container for systemd-nspawn. systemd is not running.
TODO can you actually use nspawn w/o systemd running??)");

  if (!opt.working_dir) {
    exec_print(opt, fmt::color::teal, "nspawn", args...);
    cmd_opt(opt, "systemd-nspawn", "-D", nspawn_dir, args...);
  } else {
    std::filesystem::path &working_dir_ = *opt.working_dir;
    const char *working_dir = working_dir_.c_str();

    // remove working dir from actual opt, since it is for inside nspawn
    fly::cmd_options opt_ = {
        .stdout_file = opt.stdout_file, .stderr_file = opt.stderr_file, .working_dir = std::nullopt};

    std::string dir = nspawn_dir.string();
    dir.append(opt.working_dir->string());
    fly::cmd_options opt_empty;
    exec_print(opt_empty, fmt::color::teal, "nspawn", dir.c_str(), "::", args...);

    cmd_opt(opt_, "systemd-nspawn", "-D", nspawn_dir, fmt::format("--chdir={}", working_dir), args...);
  }
}

template <typename... T>
FMT_INLINE void nspawn(T &&...args) {
  nspawn_opt({}, args...);
}

}  // namespace ymerge
