#pragma once

#include <string>

namespace Settings
{
    const std::string port { "4221" };
    inline constexpr int connectionBacklog { 5 };
    inline constexpr int nthreads { 8 };
}; // namespace Settings
