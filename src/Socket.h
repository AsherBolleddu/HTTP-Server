#pragma once

#include <cstddef>
#include <string>
#include <string_view>
class Socket
{
public:
    static constexpr int invalidFD { -1 };

private:
    int m_fd {};

public:
    explicit Socket(int fd);
    int fd() const { return m_fd; }

    std::string recvAll() const;
    std::string recvExact(std::size_t contentLength) const;
    bool sendAll(std::string_view bytes) const;

    static constexpr int getInvalidFD() { return invalidFD; }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    ~Socket();
};
