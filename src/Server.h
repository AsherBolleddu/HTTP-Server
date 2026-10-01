#pragma once

#include "Socket.h"
#include "ThreadPool.h"
#include <cstddef>
#include <string>
#include <thread>

class Server
{
private:
    Socket m_listener;
    ThreadPool m_pool;

    static Socket makeListener(const std::string& port, int connectionBacklog);

public:
    Server(const std::string& port, int connectionBacklog, std::size_t numThreads);

    static void handleClient(const Socket& client);

    void serve();
};
