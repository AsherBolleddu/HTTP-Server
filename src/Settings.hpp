#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>

namespace Settings
{
    inline const std::string port { "4221" };
    inline constexpr int connectionBacklog { 5 };
    inline constexpr int nthreads { 8 };
    inline constexpr std::size_t maxBodySize { 1024 * 1024 };
    inline const std::unordered_set<std::string_view> validSchemes { "gzip" };
}; // namespace Settings
