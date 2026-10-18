#pragma once

#include "Config.hpp"
#include "Socket.hpp"
#include "ThreadPool.hpp"

class Server
{
private:
    Config m_cfg;
    Socket m_listener;
    ThreadPool m_pool;

    static Socket makeListener();

public:
    Server(Config cfg);

    void serve();
};
