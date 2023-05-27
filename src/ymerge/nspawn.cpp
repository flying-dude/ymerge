#include <filesystem>

#include "nspawn.hpp"

using namespace std;

namespace ymerge {

void init_nspawn() {
  // TODO can you actually use nspawn w/o systemd running??
  // check if systemd is actually running, before invoking systemd-nspawn
  // we might be running inside a container or chroot, where systemd isn't started
  // https://superuser.com/questions/1017959/how-to-know-if-i-am-using-systemd-on-linux/1631444#1631444
  filesystem::path sd_booted = filesystem::path("/") / "run" / "systemd" / "system";
  if (!filesystem::is_directory(sd_booted))
    throw std::runtime_error("Cannot initialize build container for systemd-nspawn. systemd is not running.");

  todo("implement init_nspawn()");
}

}  // namespace ymerge
