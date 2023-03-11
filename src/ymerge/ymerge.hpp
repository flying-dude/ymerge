#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>

namespace ymerge {

extern const char* curated_url;

extern void sync();

}  // namespace ymerge

namespace fly {

namespace flag {
extern bool color;
extern bool confirm;
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

extern std::filesystem::path curated_aur_dir;
extern nlohmann::json whitelist;

}  // namespace fly
