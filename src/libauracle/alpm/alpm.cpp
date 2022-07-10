#include "alpm.hpp"

#include <absl/strings/ascii.h>
#include <absl/types/span.h>
#include <fmt/format.h>
#include <glob.h>

#include <fstream>
#include <stdexcept>

class Glob {
 public:
  explicit Glob(std::string_view pattern) {
    glob_ok_ = glob(std::string(pattern).c_str(), GLOB_NOCHECK, nullptr, &glob_) == 0;
    if (glob_ok_) { results_ = absl::MakeSpan(glob_.gl_pathv, glob_.gl_pathc); }
  }

  bool ok() const { return glob_ok_; }

  ~Glob() {
    if (glob_ok_) { globfree(&glob_); }
  }

  using iterator = absl::Span<char *>::iterator;
  iterator begin() { return results_.begin(); }
  iterator end() { return results_.end(); }

 private:
  glob_t glob_;
  bool glob_ok_;
  absl::Span<char *> results_;
};

class FileReader {
 public:
  explicit FileReader(const std::string &path) : file_(path) {}

  ~FileReader() {
    if (ok()) { file_.close(); }
  }

  bool GetLine(std::string &line) { return bool(std::getline(file_, line)); }

  bool ok() const { return file_.is_open(); }

 private:
  std::ifstream file_;
};

bool IsSection(std::string_view s) { return s.size() > 2 && s.front() == '[' && s.back() == ']'; }

std::pair<std::string_view, std::string_view> SplitKeyValue(std::string_view line) {
  auto equals = line.find('=');
  if (equals == line.npos) { return {line, ""}; }

  return {absl::StripTrailingAsciiWhitespace(line.substr(0, equals)),
          absl::StripLeadingAsciiWhitespace(line.substr(equals + 1))};
}

struct ParseState {
  std::string dbpath = "/var/lib/pacman";
  std::string rootdir = "/";

  std::string section;
  std::vector<std::string> repos;
};

bool ParseOneFile(const std::string &path, ParseState *state) {
  FileReader reader(path);

  for (std::string buffer; reader.GetLine(buffer);) {
    std::string_view line = absl::StripAsciiWhitespace(buffer);
    if (line.empty() || line[0] == '#') { continue; }

    if (IsSection(line)) {
      state->section = line.substr(1, line.size() - 2);
      continue;
    }

    auto [key, value] = SplitKeyValue(line);
    if (value.empty()) {
      // There aren't any directives we care about which are valueless.
      continue;
    }

    if (state->section == "options") {
      if (key == "DBPath") {
        state->dbpath = value;
      } else if (key == "RootDir") {
        state->rootdir = value;
      }
    } else {
      state->repos.emplace_back(state->section);
    }

    if (key == "Include") {
      Glob includes(value);
      if (!includes.ok()) { return false; }

      for (const auto *p : includes) {
        if (!ParseOneFile(p, state)) { return false; }
      }
    }
  }

  return reader.ok();
}

namespace alpm {

handle::handle(const std::string &config_file) {
  ParseState state;
  if (!ParseOneFile(config_file, &state))
    throw std::runtime_error(fmt::format("failed to parse config file: {}", config_file));

  alpm_errno_t err;
  alpm_handle_t *alpm = alpm_initialize("/", state.dbpath.c_str(), &err);
  if (alpm == nullptr) throw std::runtime_error(fmt::format("alpm failed with \"{}\"", alpm_strerror(err)));
  this->alpm_ = alpm;

  for (const auto &repo : state.repos) { register_syncdb(repo.c_str(), alpm_siglevel_t(0)); }
}

std::optional<alpm::pkg> db::find_satisfier(std::string name) {
  alpm_list_t *pkgcache = alpm_db_get_pkgcache(db_);
  alpm_pkg_t *pkg = alpm_find_satisfier(pkgcache, name.c_str());
  if (pkg == nullptr) { return std::nullopt; }
  return alpm::pkg{pkg};
}

}  // namespace alpm
