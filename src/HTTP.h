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
        NOT_FOUND = 404,
    };

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
    struct Response
    {
        Status status;
        std::string body;
        std::unordered_map<std::string, std::string> headers;
    };

    std::optional<RequestLine> parseRequestLine(std::string_view requestLine);
    std::optional<Request> parseRequest(std::string_view URL);

    std::string serialize(const Response& response);
    Response route(const Request& request);

    constexpr std::string_view getStatus(Status status)
    {
        using enum Status;

        switch (status)
        {
        case OK:        return "200 OK";
        case NOT_FOUND: return "404 Not Found";
        default:        return "500 Internal Server Error";
        }
    }
} // namespace HTTP
