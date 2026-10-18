#pragma once

#include "StringHash.hpp"
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace HTTP
{
    enum class Status
    {
        OK = 200,
        CREATED = 201,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        CONTENT_TOO_LARGE = 413,
        INTERNAL_SERVER_ERROR = 500,
    };

    constexpr std::string_view getStatus(HTTP::Status status)
    {
        using enum HTTP::Status;

        switch (status)
        {
        case OK:                 return "200 OK";
        case CREATED:            return "201 Created";
        case BAD_REQUEST:        return "400 Bad Request";
        case NOT_FOUND:          return "404 Not Found";
        case METHOD_NOT_ALLOWED: return "405 Method Not Allowed";
        case CONTENT_TOO_LARGE:  return "413 Content Too Large";
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
        std::unordered_map<std::string, std::string_view, StringHash, std::equal_to<>> headers;
        std::string body;
    };

    struct Response
    {
        Status status;
        std::unordered_map<std::string, std::string, StringHash, std::equal_to<>> headers;
        std::string body;
    };

    std::expected<RequestLine, Status> parseRequestLine(std::string_view requestLine);
    std::expected<std::unordered_map<std::string, std::string_view, StringHash, std::equal_to<>>, Status> parseHeaders(
        std::string_view headersSV);
    std::expected<Request, Status> parseRequest(std::string_view request);

    std::string serialize(const Response& response);

} // namespace HTTP
