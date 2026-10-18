#pragma once

#include <cstddef>
#include <string>
#include <string_view>
class Socket
{
private:
    int m_fd;

public:
    static constexpr int invalidFD { -1 };

    explicit Socket(int fd);

    int fd() const { return m_fd; }
    std::string recvAll() const;
    bool sendAll(std::string_view bytes) const;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    ~Socket();
};
