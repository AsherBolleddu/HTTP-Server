#pragma once

#include <string>
#include <string_view>
#include <unordered_set>

namespace Settings
{
    const std::string port { "4221" };
    inline constexpr int connectionBacklog { 5 };
    inline constexpr int nthreads { 8 };
    const std::unordered_set<std::string_view> validSchemes { "gzip" };
}; // namespace Settings
