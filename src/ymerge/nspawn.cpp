#include "nspawn.hpp"

#include <filesystem>
#include <iostream>

#include "cfg.hpp"
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
    filesystem::create_directories(nspawn_dir);
    init_nspawn_throws();
  } catch (const std::exception &err) {
    try {
      // delete incomplete container setup
      std::filesystem::remove_all(nspawn_dir);
    } catch (const std::exception &err2) {
      throw std::runtime_error(fmt::format("setting up nspawn failed :: {} :: {}", err.what(), err2.what()));
    }
    throw std::runtime_error(fmt::format("setting up nspawn failed: {}", err.what()));
  }
}

void init_nspawn_throws() {
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
  nspawn("useradd", "--create-home", "--groups", "wheel", "--uid", "1000", "ymerge");
}

static void cleanup_chroot() {
  info("Cleaning up chroot...");

  exec("umount", (nspawn_dir / "dev").c_str());
  exec("umount", (nspawn_dir / "proc").c_str());
  exec("umount", (nspawn_dir / "sys").c_str());
}

bool prepare_chroot_ = false;
void prepare_chroot() {
  if (prepare_chroot_) return;

  info("Preparing chroot...");

  filesystem::path from = filesystem::path("/") / "etc" / "resolv.conf";
  filesystem::path to = nspawn_dir / "etc" / "resolv.conf";

  fly::cmd_options info_opt;
  info("{} -> {}", from.c_str(), to.c_str());

  filesystem::copy_options copy_opt = filesystem::copy_options::update_existing;
  filesystem::copy(from, to, copy_opt);

  exec("mount", "--bind", "/dev", (nspawn_dir / "dev").c_str());
  exec("mount", "--bind", "/proc", (nspawn_dir / "proc").c_str());
  exec("mount", "--bind", "/sys", (nspawn_dir / "sys").c_str());

  std::atexit(cleanup_chroot);

  prepare_chroot_ = true;
}

}  // namespace ymerge
