#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace HTTP
{
    enum class Status
    {
        OK = 200,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
    };

    constexpr std::string_view getStatus(HTTP::Status status)
    {
        using enum HTTP::Status;

        switch (status)
        {
        case OK:                 return "200 OK";
        case BAD_REQUEST:        return "400 Bad Request";
        case NOT_FOUND:          return "404 Not Found";
        case METHOD_NOT_ALLOWED: return "405 Method Not Allowed";
        default:                 return "500 Internal Server Error";
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
        RequestLine reqline;
    };

    struct Response
    {
        Status status;
        std::string_view body;
        std::unordered_map<std::string, std::string> headers;
    };

    std::optional<RequestLine> parseRequestLine(std::string_view requestLine);
    std::optional<Request> parseRequest(std::string_view request);

    std::string serialize(const Response& response);

} // namespace HTTP
