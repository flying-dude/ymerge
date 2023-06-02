#pragma once

#include "ymerge_repo.hpp"

/**
 * Read ymerge configuration, initialized lazily when needed.
 * The configuration is globally available and read only once during program execution.
 * If no config file /etc/ymerge.yaml is present, a default configuration
 * will be used in that case (see config.cpp).
 */

namespace ymerge::cfg {

/// returns the git repositories listed in /etc/ymerge.json
std::vector<ymerge_repo>& git_repos();

}  // namespace ymerge::cfg
