#pragma once

#include <string>
#include <string_view>
class Socket
{
private:
    int m_fd {};

public:
    explicit Socket(int fd);
    int fd() const { return m_fd; }

    std::string recvAll() const;
    bool sendAll(std::string_view bytes) const;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    ~Socket();
};
