#include "log.hpp"

namespace ymerge {

extern std::filesystem::path nspawn_dir;
extern std::filesystem::path makepkg_dir;

extern void init_nspawn();

template <typename... T>
FMT_INLINE void nspawn_opt(fly::cmd_options opt, T &&...args) {
  exec_print(opt, fmt::color::teal, "nspawn", args...);
  cmd_opt(opt, "systemd-nspawn", "-D", nspawn_dir, args...);
}

template <typename... T>
FMT_INLINE void nspawn(T &&...args) {
  nspawn_opt({}, args...);
}

}  // namespace ymerge
