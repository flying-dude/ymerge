#include <filesystem>
#include <iostream>

#include "nspawn.hpp"

using namespace std;

namespace ymerge {

 std::filesystem::path nspawn_dir = std::filesystem::path("/") / "var" / "cache" / "ymerge" / "nspawn";

void init_nspawn() {
  // check if systemd is actually running, before invoking systemd-nspawn
  // we might be running inside a container or chroot, where systemd isn't started
  // https://superuser.com/questions/1017959/how-to-know-if-i-am-using-systemd-on-linux/1631444#1631444
  filesystem::path sd_booted = filesystem::path("/") / "run" / "systemd" / "system";
  if (!filesystem::is_directory(sd_booted))
    throw std::runtime_error(R"(Cannot initialize build container for systemd-nspawn. systemd is not running.
TODO can you actually use nspawn w/o systemd running??)");


  if (filesystem::is_directory(nspawn_dir))
    return;

  info("nspawn directory not present, creating it: {}", nspawn_dir.c_str());
  filesystem::create_directories(nspawn_dir);

  exec("pacstrap", "-c", nspawn_dir, "base", "base-devel");

  /*
   * 1) create nspawn directory
   * 2) pacstrap ("base, basel-devel")
   *    pacman.conf
   * * create [nspawn] executor
   * 3) create user ymerge, add it to wheel group
   * 4) install and configure sudo
   * 5) create build folder
   */

  /*
   * makepkg steps:
   * 1) update system (but do not sync dbs, use host dbs)
   * 2) makepkg --syncdeps
   */

  todo("implement init_nspawn()");
}

}  // namespace ymerge
