#pragma once

#include "HTTP.h"
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Handler
{
    HTTP::Response root();
    HTTP::Response echo(std::string_view body, const std::unordered_map<std::string, std::string>& reqHeaders);
    HTTP::Response userAgent(const std::unordered_map<std::string, std::string>& reqHeaders);
    HTTP::Response getFile(const std::filesystem::path& path);
    HTTP::Response postFile(std::string_view content, const std::filesystem::path& path);
} // namespace Handler
