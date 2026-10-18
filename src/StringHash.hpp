#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

struct StringHash
{
    using is_transparent = void;
    std::size_t operator()(const char* txt) const { return std::hash<std::string_view> {}(txt); }
    std::size_t operator()(const std::string& str) const { return std::hash<std::string_view> {}(str); }
    std::size_t operator()(std::string_view sv) const { return std::hash<std::string_view> {}(sv); }
};
