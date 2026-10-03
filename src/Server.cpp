#include "Server.h"
#include "FailedError.h"
#include "HTTP.h"
#include "Settings.h"
#include "Socket.h"
#include <cerrno>
#include <charconv>
#include <format>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <sys/time.h>
#include <system_error>
#include <utility>

Socket Server::makeListener(const std::string& port, int connectionBacklog)
{
    addrinfo hints {};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    auto AddressInfoDeleter { [](addrinfo* info) { freeaddrinfo(info); } };
    std::unique_ptr<addrinfo, decltype(AddressInfoDeleter)> server {};
    if (int result { getaddrinfo(nullptr, port.c_str(), &hints, std::out_ptr(server)) }; result != 0)
        throw std::runtime_error { std::format("getaddrinfo() failed. {}", gai_strerror(result)) };

    Socket listener { socket(server->ai_family, server->ai_socktype, server->ai_protocol) };
    if (listener.fd() == -1)
        throw std::runtime_error { FailedError::formattedError("socket", errno) };

    if (int reuse { 1 }; setsockopt(listener.fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        throw std::runtime_error { FailedError::formattedError("setsockopt", errno) };

    if (int dualStack { 0 }; setsockopt(listener.fd(), IPPROTO_IPV6, IPV6_V6ONLY, &dualStack, sizeof(dualStack)) == -1)
        throw std::runtime_error { FailedError::formattedError("setsockopt", errno) };

    if (bind(listener.fd(), server->ai_addr, server->ai_addrlen) == -1)
        throw std::runtime_error { FailedError::formattedError("bind", errno) };

    if (listen(listener.fd(), connectionBacklog) == -1)
        throw std::runtime_error { FailedError::formattedError("listen", errno) };

    return listener;
}

Server::Server(const std::string& port, int connectionBacklog, Config config, std::size_t numThreads)
    : m_listener { makeListener(port, connectionBacklog) }, m_config { std::move(config) },
      m_pool { [this](const Socket& client) { handleClient(client); }, numThreads }
{
}

void Server::serve()
{
    while (true)
    {
        sockaddr_storage clientInfo {};
        socklen_t clientSize { sizeof(clientInfo) };
        Socket client { accept(m_listener.fd(), reinterpret_cast<sockaddr*>(&clientInfo), &clientSize) };
        if (client.fd() == -1)
        {
            std::cerr << FailedError::formattedError("accept", errno) << '\n';
            continue;
        }

        timeval timeout { .tv_sec = 5, .tv_usec = 0 };
        if (setsockopt(client.fd(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        {
            std::cerr << FailedError::formattedError("setsockopt", errno) << '\n';
            continue;
        }

        m_pool.enqueue(std::move(client));
    }
}

void Server::sendResponse(const Socket& client, const HTTP::Response& resp) const
{
    if (!client.sendAll(HTTP::serialize(resp)))
        std::cerr << FailedError::formattedError("send", errno) << '\n';
}

void Server::handleClient(const Socket& client) const
{

    const HTTP::Response badRequest { .status = HTTP::Status::BAD_REQUEST, .body {}, .headers {} };
    bool keepAlive { true };
    while (keepAlive)
    {
        auto httpReq { client.recvAll() };
        if (httpReq.empty())
            break;

        auto req { HTTP::parseRequest(httpReq) };
        if (!req)
        {
            sendResponse(client, badRequest);
            break;
        }

        auto search { req->headers.find("connection") };
        bool connectionFound { search != req->headers.end() && search->second == "close" };
        if (connectionFound)
            keepAlive = false;

        std::size_t contentLength {};
        if (auto found { req->headers.find("content-length") }; found != req->headers.end())
        {
            std::string_view value { found->second };
            auto [ptr, ec] { std::from_chars(value.data(), value.data() + value.size(), contentLength) };
            if (ec != std::errc {} || ptr != value.data() + value.size())
            {
                sendResponse(client, badRequest);
                continue;
            }
        }

        if (contentLength > Settings::maxBodySize)
        {
            sendResponse(client, { .status = HTTP::Status::CONTENT_TOO_LARGE, .body {}, .headers {} });
            continue;
        }

        if (req->body.size() < contentLength)
        {
            std::size_t missing { contentLength - req->body.size() };
            std::string rest { client.recvExact(missing) };
            if (rest.size() != missing)
                break;
            req->body += rest;
        }

        auto resp { HTTP::route(*req, m_config.directory) };
        if (connectionFound)
            resp.headers["Connection"] = "close";

        sendResponse(client, resp);
    }
}
