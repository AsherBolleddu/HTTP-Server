#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace Settings
{
    inline const std::string port { "4221" };
    inline constexpr int connectionBacklog { 5 };
    inline constexpr int numThreads { 8 };
    inline constexpr std::size_t maxBodySize { 1024 * 1024 };
    using namespace std::literals::string_view_literals;
    inline constexpr std::array validSchemes { "gzip"sv };
} // namespace Settings
