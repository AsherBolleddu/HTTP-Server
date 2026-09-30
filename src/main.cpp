#include "FailedError.h"
#include "HTTP.h"
#include "Settings.h"
#include "Socket.h"
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::signal(SIGPIPE, SIG_IGN);

    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";
    addrinfo hints {};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    auto addrinfoDelete { [](addrinfo* addr) { freeaddrinfo(addr); } };
    std::unique_ptr<addrinfo, decltype(addrinfoDelete)> info {};
    if (int result { getaddrinfo(nullptr, Settings::port.c_str(), &hints, std::out_ptr(info)) }; result != 0)
    {
        std::cerr << std::format("getaddrinfo() failed. {}\n", gai_strerror(result));
        return 1;
    }

    Socket server { socket(info->ai_family, info->ai_socktype, info->ai_protocol) };
    if (server.fd() == -1)
    {
        std::cerr << FailedError::formattedResponse("socket", errno);
        return 1;
    }

    int reuse { 1 };
    if (setsockopt(server.fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        std::cerr << FailedError::formattedResponse("setsockopt", errno);
        return 1;
    }

    int dualStack { 0 };
    if (setsockopt(server.fd(), IPPROTO_IPV6, IPV6_V6ONLY, &dualStack, sizeof(dualStack)) == -1)
    {
        std::cerr << FailedError::formattedResponse("setsockopt", errno);
        return 1;
    }

    if (bind(server.fd(), info->ai_addr, info->ai_addrlen) == -1)
    {
        std::cerr << FailedError::formattedResponse("bind", errno);
        return 1;
    }

    if (listen(server.fd(), Settings::connectionBacklog) == -1)
    {
        std::cerr << FailedError::formattedResponse("listen", errno);
        return 1;
    }
    sockaddr_storage clientInfo {};
    socklen_t clientSize { sizeof(clientInfo) };

    while (true)
    {
        std::cout << "Waiting for a client to connect...\n";

        Socket client { accept(server.fd(), reinterpret_cast<sockaddr*>(&clientInfo), &clientSize) };
        if (client.fd() == -1)
        {
            std::cerr << FailedError::formattedResponse("accept", errno);
            return 1;
        }

        std::cout << "Client connected\n";

        auto URL { client.recvAll() };
        auto httpRequest { HTTP::parseRequest(URL) };
        if (!httpRequest)
        {
            std::cerr << "Malformed request\n";
            continue;
        }

        if (!client.sendAll(HTTP::formulateResponse(*httpRequest)))
        {
            std::cerr << FailedError::formattedResponse("send", errno);
            continue;
        }
    }

    return 0;
}
