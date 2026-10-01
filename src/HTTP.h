#pragma once

#include <optional>
#include <ostream>
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
        CONTENT_TOO_LARGE = 413,
    };

    constexpr std::string_view getStatus(Status status)
    {
        using enum Status;

        switch (status)
        {
        case OK:                return "200 OK";
        case CREATED:           return "201 Created";
        case BAD_REQUEST:       return "400 Bad Request";
        case NOT_FOUND:         return "404 Not Found";
        case CONTENT_TOO_LARGE: return "413 Content Too Large";
        default:                return "500 Internal Server Error";
        }
    }
    struct RequestLine
    {
        std::string method;
        std::string target;
        std::string version;
    };

    struct Request
    {
        RequestLine requestLine;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
    };
    struct Response
    {
        Status status;
        std::string body;
        std::unordered_map<std::string, std::string> headers;
    };

    std::optional<RequestLine> parseRequestLine(std::string_view requestLine);
    std::optional<std::unordered_map<std::string, std::string>> parseHeaders(std::string_view headerLine);
    std::optional<Request> parseRequest(std::string_view URL);

    std::string serialize(const Response& response);
    Response emptyResponse(Status status);
    Response route(const Request& request, std::string_view directory);

} // namespace HTTP
