#include "HTTP.h"
#include <optional>
#include <string>
#include <string_view>
#include <utility>

std::optional<HTTP::RequestLine> HTTP::parseRequestLine(std::string_view requestLine)
{
    auto methodLen { requestLine.find(' ') };
    if (methodLen == std::string::npos)
        return {};

    auto targetLen { requestLine.find(' ', methodLen + 1) };
    if (targetLen == std::string::npos)
        return {};

    return HTTP::RequestLine { .method { requestLine.substr(0, methodLen) },
                               .target { requestLine.substr(methodLen + 1, targetLen - methodLen - 1) },
                               .version { requestLine.substr(targetLen + 1) } };
}

// GET /index.html HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n
std::optional<HTTP::Request> HTTP::parseRequest(std::string_view URL)
{

    auto requestLineEnd { URL.find("\r\n") };
    auto headerEnd { URL.find("\r\n\r\n") };
    if (requestLineEnd == std::string_view::npos || headerEnd == std::string_view::npos)
        return {};

    std::optional<HTTP::RequestLine> requestLine { parseRequestLine(URL.substr(0, requestLineEnd)) };
    if (!requestLine)
        return {};

    return HTTP::Request { .requestLine { std::move(*requestLine) } };
}

std::string HTTP::serialize(const Response& response)
{
    std::string resp { "HTTP/1.1 " };
    resp += HTTP::getStatus(response.status);
    resp += "\r\n";

    for (const auto& [key, value] : response.headers)
        resp += key + ": " + value + "\r\n";

    resp += "\r\n" + response.body;
    return resp;
}

HTTP::Response HTTP::route(const Request& request)
{
    std::string_view route { request.requestLine.target };
    if (route == "/")
        return { .status = Status::OK, .body {}, .headers {} };

    if (route.starts_with("/echo/"))
    {
        std::string_view body { route.substr(6) };
        return { .status = Status::OK,
                 .body { body },
                 .headers { { "Content-Type", "text/plain" }, { "Content-Length", std::to_string(body.size()) } } };
    }

    return { .status = Status::NOT_FOUND, .body {}, .headers {} };
}
