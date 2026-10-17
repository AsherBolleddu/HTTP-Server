#include "Socket.hpp"
#include <array>
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

Socket::Socket(int fd) : m_fd { fd } {}

std::string Socket::recvAll() const
{
    std::string buffer {};
    std::array<char, 1024> chunk {};
    while (buffer.find("\r\n\r\n") == std::string::npos)
    {
        auto n { recv(m_fd, chunk.data(), chunk.size(), 0) };
        if (n <= 0)
            break;
        buffer.append(chunk.data(), static_cast<std::size_t>(n));
    }

    return buffer;
}

bool Socket::sendAll(std::string_view bytes) const
{
    std::ptrdiff_t bytesSent { 0 };
    while (bytesSent < std::ssize(bytes))
    {
        auto n { send(m_fd, bytes.data() + bytesSent, static_cast<std::size_t>(std::ssize(bytes) - bytesSent), 0) };
        if (n <= 0)
            break;
        bytesSent += n;
    }

    return bytesSent == std::ssize(bytes);
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
    close(m_fd);
}
