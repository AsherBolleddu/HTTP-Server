#include "Server.hpp"
#include "HTTP.hpp"
#include <cerrno>
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

namespace
{
    std::string formattedError(std::string_view fcn, int err)
    {
        return std::format("{}(). ({})", fcn, std::strerror(err));
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
    if (int result { getaddrinfo(nullptr, "4221", &hints, std::out_ptr(info)) }; result != 0)
        throw std::runtime_error { std::format("getaddrinfo() failed. ({})", gai_strerror(result)) };

    Socket listener { socket(info->ai_family, info->ai_socktype, info->ai_protocol) };
    if (listener.fd() == -1)
        throw std::runtime_error { formattedError("socket", errno) };

    int reuse { 1 };
    if (setsockopt(listener.fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        throw std::runtime_error { formattedError("setsockopt", errno) };

    int dualStack { 0 };
    if (setsockopt(listener.fd(), IPPROTO_IPV6, IPV6_V6ONLY, &dualStack, sizeof(dualStack)) == -1)
        throw std::runtime_error { formattedError("setsockopt", errno) };

    if (bind(listener.fd(), info->ai_addr, info->ai_addrlen) == -1)
        throw std::runtime_error { formattedError("bind", errno) };

    if (listen(listener.fd(), 5) == -1)
        throw std::runtime_error { formattedError("listen", errno) };

    return listener;
}

Server::Server() : m_listener { makeListener() } {}

void Server::serve() const
{
    std::cout << "Waiting for a client to connect...\n";

    Socket client { accept(m_listener.fd(), nullptr, nullptr) };
    if (client.fd() == -1)
        std::cerr << formattedError("accept", errno);

    std::cout << "Client connected\n";

    std::string httpReq { client.recvAll() };

    auto req { HTTP::parseRequest(httpReq) };
    if (!req)
        return;

    // Refactor: Make function that takes in a request, and builds the response.

    HTTP::Response resp {};
    if (req->request.method == "GET")
    {
        if (req->request.target == "/")
            resp.status = HTTP::Status::OK;
        else
            resp.status = HTTP::Status::NOT_FOUND;
    }

    std::string httpResp { HTTP::serialize(resp) };

    if (!client.sendAll(httpResp))
    {
        std::cerr << formattedError("send", errno) << '\n';
        return;
    };
}
