#pragma once

#include "HTTP.hpp"
#include <expected>
#include <optional>
#include <string>
#include <string_view>

class Router
{
private:
    struct Data
    {
        std::optional<std::string_view> info;
        const std::unordered_map<std::string, std::string_view>& header;
    };

    static std::expected<HTTP::Response, HTTP::Status> slashHandler(const Data&);
    static std::expected<HTTP::Response, HTTP::Status> echoHandler(const Data&);
    static std::expected<HTTP::Response, HTTP::Status> userAgentHandler(const Data&);

    using Handler = std::expected<HTTP::Response, HTTP::Status> (*)(const Data&);
    static inline const std::unordered_map<std::string_view, std::unordered_map<std::string_view, Handler>>
        m_dispatchTable { { "/", { { "GET", slashHandler } } },
                          { "/echo", { { "GET", echoHandler } } },
                          { "/user-agent", { { "GET", userAgentHandler } } } };

public:
    static std::expected<HTTP::Response, HTTP::Status> execute(const HTTP::Request&);
};
