#pragma once

#include <string_view>

namespace HTTP
{
    enum class Status
    {
        OK = 200,
    };

    constexpr std::string_view getStatus(HTTP::Status status)
    {
        using enum HTTP::Status;

        switch (status)
        {
        case OK: return "200 OK";
        default: return "500 Internal Server Error";
        }
    }

    struct Response
    {
        Status status;
    };
} // namespace HTTP
