#pragma once

#include "Socket.hpp"
#include "ThreadPool.hpp"

class Server
{
private:
    Socket m_listener;
    ThreadPool m_pool;

    static Socket makeListener();
    static void handleClient(const Socket& client);

public:
    Server();

    void serve();
};
