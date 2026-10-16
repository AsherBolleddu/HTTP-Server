#include "Socket.hpp"
#include <unistd.h>

Socket::Socket(int fd) : m_fd{fd} {}

Socket::~Socket() { close(m_fd); }
