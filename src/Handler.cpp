#include "Handler.h"
#include "HTTP.h"
#include "Settings.h"
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <zlib.h>

namespace
{
    std::optional<std::string_view> chooseEncoding(const std::unordered_map<std::string, std::string>& reqHeaders)
    {
        auto search { reqHeaders.find("accept-encoding") };
        if (search == reqHeaders.end())
            return {};

        using namespace std::string_view_literals;
        for (const auto& word : std::ranges::views::split(search->second, ","sv))
        {
            std::string_view scheme { word };
            if (auto first { scheme.find_first_not_of(' ') }; first != std::string_view::npos)
                scheme.remove_prefix(first);
            if (auto last { scheme.find_last_not_of(' ') }; last != std::string_view::npos)
                scheme.remove_suffix(scheme.size() - last - 1);
            if (auto result { std::ranges::find(Settings::validSchemes, scheme) };
                result != Settings::validSchemes.end())
                return *result;
        }

        return {};
    }

    std::optional<std::string> gZipCompress(std::string_view input)
    {
        z_stream stream {};
        if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
            return {};

        std::string output(deflateBound(&stream, static_cast<uLong>(input.size())), '\0');

        stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
        stream.avail_in = static_cast<uInt>(input.size());
        stream.next_out = reinterpret_cast<Bytef*>(output.data());
        stream.avail_out = static_cast<uInt>(output.size());

        int result { deflate(&stream, Z_FINISH) };
        output.resize(static_cast<std::size_t>(stream.total_out));
        deflateEnd(&stream);

        if (result != Z_STREAM_END)
            return {};

        return output;
    }

} // namespace

HTTP::Response Handler::root()
{
    return { .status = HTTP::Status::OK, .body {}, .headers {} };
}

HTTP::Response Handler::echo(std::string_view body, const std::unordered_map<std::string, std::string>& reqHeaders)
{
    HTTP::Response resp { .status = HTTP::Status::OK,
                          .body { body },
                          .headers { { "Content-Type", "text/plain" },
                                     { "Content-Length", std::to_string(body.size()) } } };

    if (auto encoding { chooseEncoding(reqHeaders) })
    {
        if (auto compressedBody { gZipCompress(body) })
        {
            resp.body = std::move(*compressedBody);
            resp.headers["Content-Encoding"] = *encoding;
            resp.headers["Content-Length"] = std::to_string(resp.body.size());
        }
    }

    return resp;
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

HTTP::Response Handler::postFile(std::string_view content, const std::filesystem::path& path)
{
    std::ofstream file { path, std::ios::binary };
    if (!file)
        return HTTP::emptyResponse(HTTP::Status::NOT_FOUND);

    file << content;
    return HTTP::emptyResponse(HTTP::Status::CREATED);
}
