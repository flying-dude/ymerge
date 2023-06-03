#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>

namespace ymerge {

extern int argc;
extern const char **argv;

/* reinvoke ymerge with identical command line but prefix with sudo.
 * the point of doing that is to obtain root permissions, if needed. */
extern void as_sudo();

void main_throws(int argc, const char **argv);

namespace flag {
extern bool color;
extern bool confirm;
extern bool makepkg;
extern bool pretend;
extern bool quiet;
extern bool remove;
extern bool srcinfo;
extern bool sync;
extern bool update;
extern bool verbose;
extern bool version;
}  // namespace flag

// derive build steps from flags and reflect the logic between them.
// f.x. flag --srcinfo entails step::install() == false
namespace step {
inline bool srcinfo() { return flag::srcinfo; }
inline bool install() { return !flag::srcinfo && !flag::remove; }
}  // namespace step

}  // namespace ymerge
