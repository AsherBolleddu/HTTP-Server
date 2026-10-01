#include "HTTP.h"
#include "Handler.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

std::optional<HTTP::RequestLine> HTTP::parseRequestLine(std::string_view requestLine)
{
    auto methodLen { requestLine.find(' ') };
    if (methodLen == std::string::npos)
        return {};

    auto targetLen { requestLine.find(' ', methodLen + 1) };
    if (targetLen == std::string::npos)
        return {};

    return HTTP::RequestLine { .method { requestLine.substr(0, methodLen) },
                               .target { requestLine.substr(methodLen + 1, targetLen - methodLen - 1) },
                               .version { requestLine.substr(targetLen + 1) } };
}

std::optional<std::unordered_map<std::string, std::string>> HTTP::parseHeaders(std::string_view headerLine)
{
    using namespace std::string_view_literals;
    std::unordered_map<std::string, std::string> headers;

    auto toLower { [](std::string_view sv) {
        std::string out { sv };
        std::ranges::transform(out, out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    } };

    for (const auto& line : std::ranges::views::split(headerLine, "\r\n"sv))
    {
        std::string_view sv { line };
        auto colon { sv.find(':') };
        if (colon == std::string_view::npos)
            return {};

        std::string key { toLower(sv.substr(0, colon)) };
        std::string_view value { sv.substr(colon + 1) };
        auto firstNonSpace { value.find_first_not_of(' ') };
        if (firstNonSpace != std::string_view::npos)
            value.remove_prefix(firstNonSpace);
        else
            value = {};

        headers[std::move(key)] = std::string { value };
    }

    return headers;
}

std::optional<HTTP::Request> HTTP::parseRequest(std::string_view URL)
{

    auto requestLineEnd { URL.find("\r\n") };
    auto headerEnd { URL.find("\r\n\r\n") };
    if (requestLineEnd == std::string_view::npos || headerEnd == std::string_view::npos)
        return {};

    std::optional<HTTP::RequestLine> requestLine { parseRequestLine(URL.substr(0, requestLineEnd)) };
    std::optional<std::unordered_map<std::string, std::string>> headers { parseHeaders(
        URL.substr(requestLineEnd + 2, headerEnd - (requestLineEnd + 2))) };
    if (!requestLine || !headers)
        return {};

    return HTTP::Request { .requestLine { std::move(*requestLine) },
                           .headers { std::move(*headers) },
                           .body { URL.substr(headerEnd + 4) } };
}

std::string HTTP::serialize(const Response& response)
{
    std::string resp { "HTTP/1.1 " };
    resp += HTTP::getStatus(response.status);
    resp += "\r\n";

    for (const auto& [key, value] : response.headers)
        resp += key + ": " + value + "\r\n";

    resp += "\r\n" + response.body;
    return resp;
}

HTTP::Response HTTP::emptyResponse(Status status)
{
    return { .status = status, .body {}, .headers {} };
}

HTTP::Response HTTP::route(const Request& request, std::string_view directory)
{
    std::string_view route { request.requestLine.target };
    if (route == "/")
        return Handler::root();

    if (route.starts_with("/echo/"))
        return Handler::echo(route.substr(6), request.headers);

    if (route.starts_with("/user-agent"))
        return Handler::userAgent(request.headers);

    if (route.starts_with("/files/"))
    {
        std::string_view fileName { route.substr(7) };
        if (!fileName.starts_with("/") && !fileName.contains(".."))
        {
            std::string_view method { request.requestLine.method };
            std::filesystem::path path { std::filesystem::path { directory } / fileName };
            if (method == "GET")
                return Handler::getFile(path);

            if (method == "POST")
                return Handler::postFile(request.body, path);
        }
    }

    return emptyResponse(HTTP::Status::NOT_FOUND);
}
