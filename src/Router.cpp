#include "Router.hpp"
#include "Config.hpp"
#include "HTTP.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
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

            return HTTP::Response {
                .status = HTTP::Status::OK,
                .headers { { "Content-Type", "text/plain" },
                           { "Content-Length", std::to_string(data.info.value().length()) } },
                .body { data.info.value() },
            };
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
