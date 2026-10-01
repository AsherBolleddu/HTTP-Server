#pragma once

#include <string>

namespace Settings
{
    inline const std::string port { "4221" };
    inline constexpr int connectionBacklog { 5 };
    inline constexpr int numThreads { 8 };
} // namespace Settings
