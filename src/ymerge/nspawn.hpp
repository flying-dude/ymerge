#include "log.hpp"

namespace ymerge {

extern std::filesystem::path nspawn_dir;

extern void init_nspawn();

template <typename... T>
FMT_INLINE void nspawn_opt(fly::cmd_options opt, T &&...args) {
  if (!opt.working_dir) {
    exec_print(opt, fmt::color::teal, "nspawn", args...);
    cmd_opt(opt, "systemd-nspawn", "-D", nspawn_dir, args...);
  } else {
    std::string& working_dir = *opt.working_dir;
    exec_print(opt, fmt::color::teal, "nspawn", working_dir, " :: ", args...);
    cmd_opt(opt, "systemd-nspawn", "-D", nspawn_dir, fmt::format("--chdir={}", working_dir), args...);
  }
}

template <typename... T>
FMT_INLINE void nspawn(T &&...args) {
  nspawn_opt({}, args...);
}

}  // namespace ymerge
