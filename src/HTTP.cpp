#include "HTTP.hpp"
#include <algorithm>
#include <cctype>
#include <expected>
#include <format>
#include <iterator>
#include <ranges>
#include <string_view>

namespace HTTP
{
    // GET /pear HTTP/1.1
    std::expected<RequestLine, Status> parseRequestLine(std::string_view requestLine)
    {
        auto methodEnd { requestLine.find(' ') };
        if (methodEnd == std::string_view::npos)
            return std::unexpected { Status::BAD_REQUEST };

        auto targetEnd { requestLine.find(' ', methodEnd + 1) };
        if (targetEnd == std::string_view::npos)
            return std::unexpected { Status::BAD_REQUEST };

        return RequestLine { .method { requestLine.substr(0, methodEnd) },
                             .target { requestLine.substr(methodEnd + 1, targetEnd - methodEnd - 1) },
                             .version { requestLine.substr(targetEnd + 1) } };
    }

    std::expected<std::unordered_map<std::string, std::string_view>, Status> parseHeaders(std::string_view headersSV)
    {
        std::unordered_map<std::string, std::string_view> headers;

        auto toLower { [](std::string_view sv) {
            std::string out { sv };
            std::ranges::transform(out, out.begin(),
                                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return out;
        } };

        using namespace std::string_view_literals;
        for (const auto line : std::ranges::views::split(headersSV, "\r\n"sv))
        {
            std::string_view headerLine { line };
            auto pos { headerLine.find(':') };
            if (pos == std::string_view::npos)
                return std::unexpected { Status::BAD_REQUEST };

            std::string key { toLower(headerLine.substr(0, pos)) };
            std::string_view value { headerLine.substr(pos + 1) };
            auto space { value.find_first_not_of(' ') };

            if (space != std::string_view::npos)
                value.remove_prefix(space);
            else
                value = "";

            headers[std::move(key)] = value;
        }

        return headers;
    }

    // GET /index.html HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n
    std::expected<Request, Status> parseRequest(std::string_view request)
    {
        auto requestLineEnd { request.find("\r\n") };
        if (requestLineEnd == std::string_view::npos)
            return std::unexpected { Status::BAD_REQUEST };

        auto requestLine { parseRequestLine(request.substr(0, requestLineEnd)) };
        if (!requestLine)
            return std::unexpected { requestLine.error() };

        auto headersEnd { request.find("\r\n\r\n", requestLineEnd + 2) };
        if (headersEnd == std::string_view::npos)
            return std::unexpected { Status::BAD_REQUEST };

        auto headers { parseHeaders(request.substr(requestLineEnd + 2, headersEnd - requestLineEnd - 2)) };
        if (!headers)
            return std::unexpected { headers.error() };

        return Request { .reqline { *requestLine },
                         .headers { std::move(*headers) },
                         .body { request.substr(headersEnd + 4) } };
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
