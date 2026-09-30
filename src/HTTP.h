#pragma once

#include <optional>
#include <string>

namespace HTTP
{
    struct RequestLine
    {
        std::string method;
        std::string target;
        std::string version;
    };

    struct Request
    {
        RequestLine requestLine;
    };

    std::optional<RequestLine> parseRequestLine(std::string_view requestLine);
    std::optional<Request> parseRequest(std::string_view URL);

    std::string formulateResponse(const Request& request);
} // namespace HTTP
