#include "log.hpp"

namespace ymerge {

std::filesystem::path chroot_dir = std::filesystem::path("/") / "var" / "cache" / "ymerge" / "nspawn";
extern void init_nspawn();

template <typename... T>
FMT_INLINE void nspawn(T &&...args) {
  init_nspawn();
  fly::cmd_options opt;
  exec_print(opt, fmt::color::medium_aquamarine, "nspawn", args...);
  todo("implement nspawn(T &&...args)");
  // cmd_opt(opt, "sudo", "chroot", chroot_dir, args...);
}

}  // namespace ymerge
