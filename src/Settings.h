#pragma once

#include <cstddef>
#include <string>

namespace Settings
{
    inline const std::string port { "4221" };
    inline constexpr int connectionBacklog { 5 };
    inline constexpr int numThreads { 8 };
    inline constexpr std::size_t maxBodySize { 1024 * 1024 };
} // namespace Settings
