#include "Socket.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// PORT: 4221
// socket -> setsockopt -> bind -> listen -> accept
// malloc -> free
// new -> delete

std::string formattedError(std::string_view fcn, int err)
{
    return std::format("{}() failed. {}\n", fcn, std::strerror(err));
}

int main()
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";

    addrinfo hints {};
    hints.ai_family = AF_INET6;      // IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // Support IPv6 and IPv4

    auto addrInfoDeleter { [](addrinfo* info) { freeaddrinfo(info); } };
    std::unique_ptr<addrinfo, decltype(addrInfoDeleter)> info {};

    if (int result { getaddrinfo(nullptr, "4221", &hints, std::out_ptr(info)) }; result != 0)
    {
        std::cerr << std::format("getaddrinfo() failed. {}\n", gai_strerror(result));
        return EXIT_FAILURE;
    }

    Socket listener { socket(info->ai_family, info->ai_socktype, info->ai_protocol) };
    if (listener.fd() == -1)
    {
        std::cerr << formattedError("socket", errno);
        return EXIT_FAILURE;
    }

    int reuse { 1 };
    if (setsockopt(listener.fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        std::cerr << formattedError("setsockopt", errno);
        return EXIT_FAILURE;
    }

    int dualStack { 0 };
    if (setsockopt(listener.fd(), IPPROTO_IPV6, IPV6_V6ONLY, &dualStack, sizeof(dualStack)) == -1)
    {
        std::cerr << formattedError("setsockopt", errno);
        return EXIT_FAILURE;
    }

    if (bind(listener.fd(), info->ai_addr, info->ai_addrlen) == -1)
    {
        std::cerr << formattedError("bind", errno);
        return EXIT_FAILURE;
    }

    constexpr int connectionBacklog { 5 };
    if (listen(listener.fd(), connectionBacklog) == -1)
    {
        std::cerr << formattedError("listen", errno);
        return EXIT_FAILURE;
    }

    std::cout << "Waiting for a client to connect...\n";

    Socket client { accept(listener.fd(), nullptr, nullptr) };
    if (client.fd() == -1)
    {
        std::cerr << formattedError("accept", errno);
        return EXIT_FAILURE;
    }

    std::cout << "Client connected\n";

    return 0;
}
