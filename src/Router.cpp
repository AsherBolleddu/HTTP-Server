#include "Router.hpp"
#include "HTTP.hpp"
#include <expected>
#include <optional>
#include <string>
#include <string_view>

std::expected<HTTP::Response, HTTP::Status> Router::slashHandler(std::optional<std::string_view>)
{
    return HTTP::Response { .status = HTTP::Status::OK };
}

// /echo/abc
std::expected<HTTP::Response, HTTP::Status> Router::echoHandler(std::optional<std::string_view> req)
{
    if (!req)
        return std::unexpected { HTTP::Status::NOT_FOUND };

    return HTTP::Response { .status = HTTP::Status::OK,
                            .body { *req },
                            .headers { { "Content-Type", "text/plain" },
                                       { "Content-Length", std::to_string(req->length()) } } };
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

    return innerIt->second(pos == std::string_view::npos ? std::nullopt
                                                         : std::optional { req.reqline.target.substr(pos + 1) });
}
