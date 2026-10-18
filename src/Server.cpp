#include "Server.hpp"
#include "Config.hpp"
#include "HTTP.hpp"
#include "Router.hpp"
#include "Settings.hpp"
#include <cerrno>
#include <charconv>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <system_error>
#include <utility>

namespace
{
    std::string formattedError(std::string_view fcn, int err)
    {
        return std::format("{}(). ({})", fcn, std::strerror(err));
    }

    void sendResponse(const Socket& client, const HTTP::Response& resp)
    {
        if (!client.sendAll(HTTP::serialize(resp)))
            std::cerr << formattedError("send", errno) << '\n';
    }

    void handleClient(const Socket& client, const Config& cfg)
    {
        while (true)
        {
            std::string httpReq { client.recvAll() };

            auto req { HTTP::parseRequest(httpReq) };
            if (!req)
            {
                sendResponse(client, { .status = req.error() });
                return;
            }

            if (auto it { req->headers.find("content-length") }; it != req->headers.end())
            {
                std::size_t len;
                const auto [ptr, ec] { std::from_chars(it->second.data(), it->second.data() + it->second.size(), len) };
                if (ec != std::errc {} || ptr != it->second.data() + it->second.size())
                {
                    sendResponse(client, { .status = HTTP::Status::BAD_REQUEST });
                    return;
                }

                req->body += client.recvExact(len - req->body.size());
            }

            auto resp { Router::execute(*req, cfg) };
            if (!resp)
            {
                sendResponse(client, { .status = resp.error() });
                return;
            }

            sendResponse(client, *resp);
        }
    }
} // namespace

Socket Server::makeListener()
{
    addrinfo hints {};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    auto addrInfoDeleter { [](addrinfo* info) { freeaddrinfo(info); } };
    std::unique_ptr<addrinfo, decltype(addrInfoDeleter)> info {};
    if (int result { getaddrinfo(nullptr, Settings::port.c_str(), &hints, std::out_ptr(info)) }; result != 0)
        throw std::runtime_error { std::format("getaddrinfo() failed. ({})", gai_strerror(result)) };

    Socket listener { socket(info->ai_family, info->ai_socktype, info->ai_protocol) };
    if (listener.fd() == -1)
        throw std::runtime_error { formattedError("socket", errno) };

    int reuse { 1 };
    if (setsockopt(listener.fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        throw std::runtime_error { formattedError("setsockopt", errno) };

    struct timeval timeout { .tv_sec = 5, .tv_usec = 0 };
    if (setsockopt(listener.fd(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        throw std::runtime_error { formattedError("setsockopt", errno) };

    int dualStack { 0 };
    if (setsockopt(listener.fd(), IPPROTO_IPV6, IPV6_V6ONLY, &dualStack, sizeof(dualStack)) == -1)
        throw std::runtime_error { formattedError("setsockopt", errno) };

    if (bind(listener.fd(), info->ai_addr, info->ai_addrlen) == -1)
        throw std::runtime_error { formattedError("bind", errno) };

    if (listen(listener.fd(), Settings::connectionBacklog) == -1)
        throw std::runtime_error { formattedError("listen", errno) };

    return listener;
}

Server::Server(Config cfg)
    : m_cfg { std::move(cfg) }, m_listener { makeListener() },
      m_pool { Settings::nthreads, [&cfg = m_cfg](const Socket& client) { handleClient(client, cfg); } }
{
}

void Server::serve()
{
    while (true)
    {
        Socket client { accept(m_listener.fd(), nullptr, nullptr) };
        if (client.fd() == -1)
            std::cerr << formattedError("accept", errno);

        m_pool.enqueue(std::move(client));
    }
}
