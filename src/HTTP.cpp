#include "HTTP.hpp"
#include <format>
#include <iterator>
#include <optional>
#include <string_view>

namespace HTTP
{
    // GET /pear HTTP/1.1
    std::optional<RequestLine> parseRequestLine(std::string_view requestLine)
    {
        auto methodEnd { requestLine.find(' ') };
        if (methodEnd == std::string_view::npos)
            return std::nullopt;

        auto targetEnd { requestLine.find(' ', methodEnd + 1) };
        if (targetEnd == std::string_view::npos)
            return std::nullopt;

        return RequestLine { .method { requestLine.substr(0, methodEnd) },
                             .target { requestLine.substr(methodEnd + 1, targetEnd - methodEnd - 1) },
                             .version { requestLine.substr(targetEnd + 1) } };
    }

    // GET /index.html HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n
    std::optional<Request> parseRequest(std::string_view request)
    {
        auto requestLineEnd { request.find("\r\n") };
        if (requestLineEnd == std::string_view::npos)
            return std::nullopt;

        auto requestLine { parseRequestLine(request.substr(0, requestLineEnd)) };
        if (!requestLine)
            return std::nullopt;

        return Request { .reqline { *requestLine } };
    }

    std::string serialize(const Response& response)
    {
        std::string output { "HTTP/1.1 " };
        output += getStatus(response.status);
        for (const auto& [key, value] : response.headers)
            std::format_to(std::back_inserter(output), "\r\n{}: {}", key, value);

        output += "\r\n\r\n";
        output += response.body;
        return output;
    }

} // namespace HTTP
