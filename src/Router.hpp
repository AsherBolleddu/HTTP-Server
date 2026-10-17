#pragma once

#include "HTTP.hpp"
#include <expected>
#include <optional>
#include <string_view>

class Router
{
private:
    static std::expected<HTTP::Response, HTTP::Status> slashHandler(std::optional<std::string_view>);
    static std::expected<HTTP::Response, HTTP::Status> echoHandler(std::optional<std::string_view>);

    using Handler = std::expected<HTTP::Response, HTTP::Status> (*)(std::optional<std::string_view>);
    static inline const std::unordered_map<std::string_view, std::unordered_map<std::string_view, Handler>>
        m_dispatchTable { { "/", { { "GET", slashHandler } } }, { "/echo", { { "GET", echoHandler } } } };

public:
    static std::expected<HTTP::Response, HTTP::Status> execute(const HTTP::Request&);
};
