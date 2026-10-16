#pragma once

#include "Socket.hpp"

class Server
{
private:
    Socket m_listener;

    static Socket makeListener();

public:
    Server();

    void serve() const;
};
