#include "nspawn.hpp"

#include <filesystem>
#include <iostream>

#include "file_util.hpp"

using namespace std;
using namespace fly;

namespace ymerge {

std::filesystem::path nspawn_dir = std::filesystem::path("/") / "var" / "cache" / "ymerge" / "nspawn";

void init_nspawn_throws();
void init_nspawn() {
  if (filesystem::exists(nspawn_dir)) return;

  try {
    info("nspawn directory not present, creating it: {}", nspawn_dir.c_str());
    filesystem::create_directories(nspawn_dir / "makepkg");  // create together with makepkg folder
    init_nspawn_throws();
  } catch (const std::exception &err) {
    try {
      // delete incomplete container setup
      std::filesystem::remove_all(nspawn_dir);
    } catch (const std::exception &err2) {
      throw std::runtime_error(fmt::format("creating nspawn failed :: {} :: {}", err.what(), err2.what()));
    }
    throw std::runtime_error(fmt::format("creating nspawn failed: {}", err.what()));
  }
}

void init_nspawn_throws() {
  // check if systemd is actually running, before invoking systemd-nspawn
  // we might be running inside a container or chroot, where systemd isn't started
  // https://superuser.com/questions/1017959/how-to-know-if-i-am-using-systemd-on-linux/1631444#1631444
  filesystem::path sd_booted = filesystem::path("/") / "run" / "systemd" / "system";
  if (!filesystem::is_directory(sd_booted))
    throw std::runtime_error(R"(Cannot initialize build container for systemd-nspawn. systemd is not running.
TODO can you actually use nspawn w/o systemd running??)");

  // use pacstrap to set up the base system for the container
  exec("pacstrap", "-c", nspawn_dir, "base", "base-devel", "sudo");

  // configure pacman
  fly::file::write(nspawn_dir / "etc" / "pacman.conf", R"(
[options]
Architecture = auto

Color

# By default, pacman accepts packages signed by keys that its local keyring
# trusts (see pacman-key and its man page), as well as unsigned packages.
SigLevel    = Required DatabaseOptional
LocalFileSigLevel = Optional

[core]
Include = /etc/pacman.d/mirrorlist

[extra]
Include = /etc/pacman.d/mirrorlist

[community]
Include = /etc/pacman.d/mirrorlist
)");

  // configure sudo
  fly::file::write(nspawn_dir / "etc" / "sudoers", R"(
Defaults    !env_reset
root ALL=(ALL) ALL
%wheel ALL=(ALL) NOPASSWD: ALL
)");

  // creating a user, since makepkg will complain if we run it as root
  nspawn("useradd", "--create-home", "--groups", "wheel", "ymerge");

  /*
   * makepkg steps:
   * 1) bind-mounts:
   *   * bind-mount: /var/cache/pacman/pkg
   *   * bind-mount: /var/lib/pacman/sync
   *   * use atexit to unmount: https://cplusplus.com/reference/cstdlib/atexit/
   *   * use ymerge lockfile to prevent remount (wrap entire makepkg step into locking step)
   * 2) update the system (but do not sync dbs, use host dbs)
   * 3) makepkg --syncdeps (without install, use nspawn_opt to change working dir)
   */

  todo("implement init_nspawn()");
}

}  // namespace ymerge
