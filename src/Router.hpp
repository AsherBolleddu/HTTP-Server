#pragma once

#include "Config.hpp"
#include "HTTP.hpp"
#include <expected>

namespace Router
{

    std::expected<HTTP::Response, HTTP::Status> execute(const HTTP::Request&, const Config& cfg);

} // namespace Router
