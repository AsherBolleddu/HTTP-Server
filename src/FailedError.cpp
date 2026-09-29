#include "FailedError.h"
#include <cstring>
#include <format>

std::string FailedError::formattedResponse(std::string_view function, int error)
{
    return std::format("{}() failed. {}\n", function, std::strerror(error));
}
