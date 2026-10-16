#pragma once

#include <cstddef>
class Socket
{
private:
    static constexpr int invalidFD { -1 };
    int m_fd;

public:
    explicit Socket(int fd);

    int fd() const { return m_fd; }
    std::ptrdiff_t sendAll() const;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    ~Socket();
};
