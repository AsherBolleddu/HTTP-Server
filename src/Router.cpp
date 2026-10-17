#include "Router.hpp"
#include "HTTP.hpp"
#include <expected>
#include <optional>
#include <string>
#include <string_view>

std::expected<HTTP::Response, HTTP::Status> Router::slashHandler(const Data&)
{
    return HTTP::Response { .status = HTTP::Status::OK };
}

// /echo/abc
std::expected<HTTP::Response, HTTP::Status> Router::echoHandler(const Data& data)
{
    if (!data.info)
        return std::unexpected { HTTP::Status::NOT_FOUND };

    return HTTP::Response {
        .status = HTTP::Status::OK,
        .headers { { "Content-Type", "text/plain" }, { "Content-Length", std::to_string(data.info.value().length()) } },
        .body { data.info.value() },
    };
}

std::expected<HTTP::Response, HTTP::Status> Router::userAgentHandler(const Data& data)
{
    auto userAgent { data.header.find("user-agent") };
    if (userAgent == data.header.end())
        return std::unexpected { HTTP::Status::BAD_REQUEST };

    return HTTP::Response { .status = HTTP::Status::OK,
                            .headers { { "Content-Type", "text/plain" },
                                       { "Content-Length", std::to_string(userAgent->second.length()) } },
                            .body { userAgent->second } };
}

std::expected<HTTP::Response, HTTP::Status> Router::execute(const HTTP::Request& req)
{
    auto pos { req.reqline.target.find('/', 1) };
    auto outerIt { m_dispatchTable.find(req.reqline.target.substr(0, pos)) };
    if (outerIt == m_dispatchTable.end())
        return std::unexpected { HTTP::Status::NOT_FOUND };

    auto innerIt { outerIt->second.find(req.reqline.method) };
    if (innerIt == outerIt->second.end())
        return std::unexpected { HTTP::Status::METHOD_NOT_ALLOWED };

    return innerIt->second(
        { .info { pos == std::string_view::npos ? std::nullopt : std::optional { req.reqline.target.substr(pos + 1) } },
          .header { req.headers } });
}
