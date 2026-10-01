#include "Handler.h"
#include "HTTP.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

HTTP::Response Handler::root()
{
    return { .status = HTTP::Status::OK, .body {}, .headers {} };
}

HTTP::Response Handler::echo(std::string_view body)
{
    return { .status = HTTP::Status::OK,
             .body { body },
             .headers { { "Content-Type", "text/plain" }, { "Content-Length", std::to_string(body.size()) } } };
}

HTTP::Response Handler::userAgent(const std::unordered_map<std::string, std::string>& reqHeaders)
{
    auto search { reqHeaders.find("user-agent") };
    if (search == reqHeaders.end())
        return HTTP::emptyResponse(HTTP::Status::NOT_FOUND);

    return { .status = HTTP::Status::OK,
             .body { search->second },
             .headers { { "Content-Type", "text/plain" },
                        { "Content-Length", std::to_string(search->second.size()) } } };
}

HTTP::Response Handler::getFile(const std::filesystem::path& path)
{
    if (!std::filesystem::is_regular_file(path))
        return HTTP::emptyResponse(HTTP::Status::NOT_FOUND);

    std::ifstream file { path, std::ios::binary };
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content { buffer.str() };
    auto contentSize { std::to_string(content.size()) };
    return { .status = HTTP::Status::OK,
             .body { std::move(content) },
             .headers { { "Content-Type", "application/octet-stream" },
                        { "Content-Length", std::move(contentSize) } } };
}

HTTP::Response Handler::postFile(const std::filesystem::path& path, std::string_view content)
{
    std::ofstream file { path, std::ios::binary };
    if (!file)
        return HTTP::emptyResponse(HTTP::Status::NOT_FOUND);

    file << content;
    return HTTP::emptyResponse(HTTP::Status::CREATED);
}
