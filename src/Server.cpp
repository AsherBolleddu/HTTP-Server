#include "Server.h"
#include "FailedError.h"
#include "HTTP.h"
#include "Socket.h"
#include <cerrno>
#include <format>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>

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
    sockaddr_storage clientInfo {};
    socklen_t clientSize { sizeof(clientInfo) };
    while (true)
    {
        Socket client { accept(m_listener.fd(), reinterpret_cast<sockaddr*>(&clientInfo), &clientSize) };
        if (client.fd() == -1)
        {
            std::cerr << FailedError::formattedError("accept", errno) << '\n';
            continue;
        }

        m_pool.enqueue(std::move(client));
    }
}

void Server::handleClient(const Socket& client) const
{
    auto httpReq { client.recvAll() };
    auto req { HTTP::parseRequest(httpReq) };
    if (!req)
        return;

    auto resp { HTTP::route(*req, m_config.directory) };
    auto httpResp { HTTP::serialize(resp) };

    if (!client.sendAll(httpResp))
    {
        std::cerr << FailedError::formattedError("send", errno) << '\n';
        return;
    }
}
