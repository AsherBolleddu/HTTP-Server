#include "HTTP.h"
#include <optional>
#include <string_view>

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

std::string HTTP::formulateResponse(const Request& request)
{
    std::string response { "HTTP/1.1" };
    if (request.requestLine.target != "/")
        response += " 404 Not Found";
    else
        response += " 200 OK";

    response += "\r\n\r\n";
    return response;
}
