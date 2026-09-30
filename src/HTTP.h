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
        NOT_FOUND = 404,
    };

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

        friend std::ostream& operator<<(std::ostream& out, const Request& req)
        {
            const auto& [requestLine, headers] { req };
            const auto& [method, target, version] { requestLine };
            out << "Method: " << method << '\n';
            out << "Target: " << target << '\n';
            out << "Version: " << version << '\n';
            out << "Headers:\n";
            for (const auto& [key, value] : headers)
                out << key << ": " << value << '\n';

            return out;
        }
    };
    struct Response
    {
        Status status;
        std::string body;
        std::unordered_map<std::string, std::string> headers;

        friend std::ostream& operator<<(std::ostream& out, const Response& resp)
        {
            const auto& [status, body, headers] { resp };
            out << "Status: " << getStatus(status) << '\n';
            out << "Body: " << body << '\n';
            out << "Headers:\n";
            for (const auto& [key, value] : headers)
                out << key << ": " << value << '\n';

            return out;
        }
    };

    std::optional<RequestLine> parseRequestLine(std::string_view requestLine);
    std::optional<std::unordered_map<std::string, std::string>> parseHeaders(std::string_view headerLine);
    std::optional<Request> parseRequest(std::string_view URL);

    std::string serialize(const Response& response);
    Response route(const Request& request);

} // namespace HTTP
