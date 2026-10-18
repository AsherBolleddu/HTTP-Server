#include "Router.hpp"
#include "Config.hpp"
#include "HTTP.hpp"
#include "Settings.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <zlib.h>

namespace
{
    std::optional<std::string_view> chooseEncoding(const std::unordered_map<std::string, std::string_view>& headers)
    {
        using namespace std::string_view_literals;
        auto it(headers.find("accept-encoding"));
        if (it == headers.end())
            return std::nullopt;

        for (const auto line : std::ranges::views::split(it->second, ","sv))
        {
            std::string_view scheme { line };
            if (auto first { scheme.find_first_not_of(' ') }; first != std::string_view::npos)
                scheme.remove_prefix(first);
            if (auto last { scheme.find_last_not_of(' ') }; last != std::string_view::npos)
                scheme.remove_suffix(scheme.size() - last - 1);
            if (Settings::validSchemes.contains(scheme))
                return scheme;
        }

        return std::nullopt;
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
namespace Router
{
    namespace
    {
        struct Data
        {
            std::optional<std::string_view> info;
            const std::unordered_map<std::string, std::string_view>& header;
            std::string_view body;
            const Config& cfg;
        };

        std::expected<HTTP::Response, HTTP::Status> slashHandler(const Data&)
        {
            return HTTP::Response { .status = HTTP::Status::OK };
        }

        // /echo/abc
        std::expected<HTTP::Response, HTTP::Status> echoHandler(const Data& data)
        {
            if (!data.info)
                return std::unexpected { HTTP::Status::NOT_FOUND };

            HTTP::Response resp {
                .status = HTTP::Status::OK,
                .headers { { "Content-Type", "text/plain" },
                           { "Content-Length", std::to_string(data.info.value().length()) } },
                .body { data.info.value() },
            };

            auto encoding { chooseEncoding(data.header) };
            if (!encoding)
                return resp;

            auto newData { gZipCompress(data.info.value()) };
            if (!newData)
                return resp;

            resp.headers.insert({ "Content-Encoding", std::string { *encoding } });
            resp.body = std::move(*newData);
            resp.headers["Content-Length"] = std::to_string(resp.body.length());
            return resp;
        }

        std::expected<HTTP::Response, HTTP::Status> userAgentHandler(const Data& data)
        {
            auto userAgent { data.header.find("user-agent") };
            if (userAgent == data.header.end())
                return std::unexpected { HTTP::Status::BAD_REQUEST };

            return HTTP::Response { .status = HTTP::Status::OK,
                                    .headers { { "Content-Type", "text/plain" },
                                               { "Content-Length", std::to_string(userAgent->second.length()) } },
                                    .body { userAgent->second } };
        }

        std::expected<HTTP::Response, HTTP::Status> getFilesHandler(const Data& data)
        {
            if (data.cfg.directory.empty())
                return std::unexpected { HTTP::Status::NOT_FOUND };

            if (!data.info)
                return std::unexpected { HTTP::Status::NOT_FOUND };

            namespace fs = std::filesystem;
            fs::path path { fs::path { data.cfg.directory } / data.info.value() };
            if (!fs::is_regular_file(path))
                return std::unexpected { HTTP::Status::NOT_FOUND };

            std::ifstream file { path, std::ios::binary };
            if (!file)
                std::unexpected { HTTP::Status::INTERNAL_SERVER_ERROR };

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content { buffer.str() };
            return HTTP::Response { .status = HTTP::Status::OK,
                                    .headers { { "Content-Type", "application/octet-stream" },
                                               { "Content-Length", std::to_string(content.length()) } },
                                    .body { std::move(content) } };
        }

        std::expected<HTTP::Response, HTTP::Status> postFilesHandler(const Data& data)
        {
            if (data.cfg.directory.empty())
                return std::unexpected { HTTP::Status::NOT_FOUND };

            if (!data.info)
                return std::unexpected { HTTP::Status::NOT_FOUND };

            namespace fs = std::filesystem;
            fs::path path { fs::path { data.cfg.directory } / data.info.value() };

            std::ofstream file { path, std::ios::binary };
            file << data.body;

            return HTTP::Response { .status = HTTP::Status::CREATED };
        }

        using Handler = std::expected<HTTP::Response, HTTP::Status> (*)(const Data&);
        const std::unordered_map<std::string_view, std::unordered_map<std::string_view, Handler>> dispatchTable {
            { "/", { { "GET", slashHandler } } },
            { "/echo", { { "GET", echoHandler } } },
            { "/user-agent", { { "GET", userAgentHandler } } },
            { "/files", { { "GET", getFilesHandler }, { "POST", postFilesHandler } } }
        };
    } // namespace

    std::expected<HTTP::Response, HTTP::Status> execute(const HTTP::Request& req, const Config& cfg)
    {
        auto pos { req.reqline.target.find('/', 1) };
        auto outerIt { dispatchTable.find(req.reqline.target.substr(0, pos)) };
        if (outerIt == dispatchTable.end())
            return std::unexpected { HTTP::Status::NOT_FOUND };

        auto innerIt { outerIt->second.find(req.reqline.method) };
        if (innerIt == outerIt->second.end())
            return std::unexpected { HTTP::Status::METHOD_NOT_ALLOWED };

        return innerIt->second(
            { .info { pos == std::string_view::npos ? std::nullopt
                                                    : std::optional { req.reqline.target.substr(pos + 1) } },
              .header { req.headers },
              .body { req.body },
              .cfg { cfg } });
    }

} // namespace Router
