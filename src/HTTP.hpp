#pragma once

#include <expected>
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
        INTERNAL_SERVER_ERROR = 500,
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
        std::unordered_map<std::string, std::string_view> headers;
    };

    struct Response
    {
        Status status;
        std::unordered_map<std::string_view, std::string> headers;
        std::string body;
    };

    std::expected<RequestLine, Status> parseRequestLine(std::string_view requestLine);
    std::expected<std::unordered_map<std::string, std::string_view>, Status> parseHeaders(std::string_view headersSV);
    std::expected<Request, Status> parseRequest(std::string_view request);

    std::string serialize(const Response& response);

} // namespace HTTP
