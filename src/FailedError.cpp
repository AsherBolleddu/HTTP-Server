#include "FailedError.h"
#include <cstring>
#include <format>

std::string FailedError::formattedError(std::string_view function, int error)
{
    return std::format("{}() failed. {}", function, std::strerror(error));
}
