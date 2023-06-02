#pragma once

#include "cfg.hpp"
#include "log.hpp"
#include "ymerge.hpp"

namespace ymerge {

extern std::filesystem::path nspawn_dir;

void prepare_chroot();
void init_nspawn();

template <typename... T>
FMT_INLINE void nspawn_opt(fly::cmd_options opt, T &&...args) {
  if (opt.working_dir.has_value()) {
    throw std::runtime_error(R"(cannot use working directory inside nspawn. use the pattern below to switch dir:
==> sh -c "cd /my/working/dir; my_cmd arg1 arg2 ...")");
  }

  if (system("which systemd-nspawn > /dev/null 2>&1") != 0) ymerge::flag::chroot = true;

  // check if systemd is actually running, before invoking systemd-nspawn
  // we might be running inside a container or chroot, where systemd isn't started
  // https://superuser.com/questions/1017959/how-to-know-if-i-am-using-systemd-on-linux/1631444#1631444
  std::filesystem::path sd_booted = std::filesystem::path("/") / "run" / "systemd" / "system";
  if (!ymerge::flag::chroot && !std::filesystem::is_directory(sd_booted)) {
    warn("cannot initialize build container for systemd-nspawn. systemd is not running.");
    ymerge::flag::chroot = true;
  }

  exec_print(opt, fmt::color::teal, ymerge::flag::chroot ? "chroot" : "nspawn", args...);

  if (!ymerge::flag::chroot) {
    cmd_opt(opt, "systemd-nspawn", "-D", nspawn_dir, args...);
  } else {
    prepare_chroot();
    cmd_opt(opt, "chroot", nspawn_dir, args...);
  }
}

template <typename... T>
FMT_INLINE void nspawn(T &&...args) {
  nspawn_opt({}, args...);
}

}  // namespace ymerge
