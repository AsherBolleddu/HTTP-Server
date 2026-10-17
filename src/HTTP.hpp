#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace HTTP
{
    enum class Status
    {
        OK = 200,
        NOT_FOUND = 404,
    };

    constexpr std::string_view getStatus(HTTP::Status status)
    {
        using enum HTTP::Status;

        switch (status)
        {
        case OK:        return "200 OK";
        case NOT_FOUND: return "404 Not Found";
        default:        return "500 Internal Server Error";
        }
    }

    struct RequestLine
    {
        std::string_view method;
        std::string_view target;
        std::string_view version;
    };

    struct Request
    {
        RequestLine request;
    };

    struct Response
    {
        Status status;
    };

    std::optional<RequestLine> parseRequestLine(std::string_view requestLine);
    std::optional<Request> parseRequest(std::string_view request);

    std::string serialize(const Response& response);

} // namespace HTTP
