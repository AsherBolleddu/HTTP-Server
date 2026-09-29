#pragma once

class Socket {
private:
  int m_fd{};

public:
  explicit Socket(int fd);
  int fd() { return m_fd; }

  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;
  ~Socket();
};
