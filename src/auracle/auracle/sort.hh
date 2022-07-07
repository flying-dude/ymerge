// SPDX-License-Identifier: MIT
#pragma once

#include <functional>
#include <string_view>

#include "aur/package.hh"

namespace sort {

enum class OrderBy : int8_t { ORDER_ASC, ORDER_DESC };

using Sorter = std::function<bool(const aur::Package&, const aur::Package&)>;

// Returns a binary predicate suitable for use with std::sort.
Sorter MakePackageSorter(std::string_view field, OrderBy order_by);

}  // namespace sort
