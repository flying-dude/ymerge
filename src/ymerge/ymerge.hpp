#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>

namespace ymerge {

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
