#include "Socket.h"
#include <algorithm>
#include <cstddef>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

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

std::string Socket::recvExact(std::size_t contentLength) const
{
    std::string body {};
    std::string chunk(1024, '\0');
    while (body.size() < contentLength)
    {
        auto bytesRecieved { recv(m_fd, chunk.data(), std::min(chunk.size(), contentLength - body.size()), 0) };
        if (bytesRecieved <= 0)
            break;
        body.append(chunk.data(), static_cast<std::size_t>(bytesRecieved));
    }

    return body;
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

Socket::Socket(Socket&& other) noexcept : m_fd { std::exchange(other.m_fd, invalidFD) } {}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this == &other)
        return *this;

    if (m_fd != invalidFD)
        close(m_fd);

    m_fd = std::exchange(other.m_fd, invalidFD);
    return *this;
}

Socket::~Socket()
{
    if (m_fd != invalidFD)
        close(m_fd);
}
