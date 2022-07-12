#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <xresult.hpp>

namespace fly {

namespace flag {
extern bool color;
extern bool quiet;
extern bool remove;
extern bool srcinfo;
extern bool sync;
extern bool verbose;
}  // namespace flag

// derive build steps from flags and reflect the logic between them.
// f.x. flag --srcinfo entails step::install() == false
namespace step {
inline bool srcinfo() { return flag::srcinfo; }
inline bool install() { return !flag::srcinfo && !flag::remove; }
inline bool remove() { return flag::remove; }
}  // namespace step

extern std::filesystem::path cache_dir;
extern std::filesystem::path repo_dir;
extern std::filesystem::path git_dir;
extern std::filesystem::path pkg_dir;
extern nlohmann::json whitelist;

}  // namespace fly
