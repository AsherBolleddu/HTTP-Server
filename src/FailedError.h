#pragma once

#include <string>
#include <string_view>

namespace FailedError
{
    std::string formattedError(std::string_view function, int error);
} // namespace FailedError
