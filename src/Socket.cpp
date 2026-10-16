#include "Socket.hpp"
#include <cstddef>
#include <iterator>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

Socket::Socket(int fd) : m_fd { fd } {}

std::ptrdiff_t Socket::sendAll() const
{
    std::string response { "HTTP/1.1 200 OK\r\n\r\n" };
    std::ptrdiff_t bytesSent { 0 };
    while (bytesSent < std::ssize(response))
    {
        auto n { send(m_fd, response.data() + bytesSent, response.size() - static_cast<std::size_t>(bytesSent), 0) };
        if (n <= 0)
            break;
        bytesSent += n;
    }

    return bytesSent;
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
