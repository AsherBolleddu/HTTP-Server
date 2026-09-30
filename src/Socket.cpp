#include "Socket.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>

Socket::Socket(int fd) : m_fd { fd } {}

std::string Socket::recvAll() const
{
    std::string buffer {};
    std::string chunk(1024, '\0');

    while (buffer.find("\r\n\r\n") == std::string::npos)
    {
        auto bytesReceived { recv(m_fd, chunk.data(), chunk.size(), 0) };
        if (bytesReceived <= 0)
            break;
        buffer.append(chunk.data(), static_cast<std::size_t>(bytesReceived));
    }

    return buffer;
}

bool Socket::sendAll(std::string_view bytes) const
{
    std::size_t sent { 0 };
    while (sent < bytes.size())
    {
        auto bytesSent { send(m_fd, bytes.data() + sent, bytes.size() - sent, 0) };
        if (bytesSent <= 0)
            return false;
        sent += static_cast<std::size_t>(bytesSent);
    }

    return true;
}

Socket::~Socket()
{
    close(m_fd);
}
