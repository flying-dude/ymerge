// SPDX-License-Identifier: MIT
#pragma once

#include <set>
#include <string_view>
#include <vector>

#include "aur/package.hh"

namespace auracle {

enum class DependencyKind : int {
  Depend,
  MakeDepend,
  CheckDepend,
};

bool ParseDependencyKinds(std::string_view input,
                          std::set<DependencyKind>* dependency_kinds);

const std::vector<aur::Dependency>& GetDependenciesByKind(
    const aur::Package* package, DependencyKind kind);

}  // namespace auracle
