#pragma once

#include <string>
#include <string_view>

namespace FailedError
{
    std::string formattedResponse(std::string_view function, int error);
} // namespace FailedError
