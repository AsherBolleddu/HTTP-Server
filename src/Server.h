#pragma once

#include "Config.h"
#include "HTTP.h"
#include "Socket.h"
#include "ThreadPool.h"
#include <cstddef>
#include <string>
#include <thread>

class Server
{
private:
    Socket m_listener;
    Config m_config;
    ThreadPool m_pool;

    static Socket makeListener(const std::string& port, int connectionBacklog);
    void sendResponse(const Socket& client, const HTTP::Response& resp) const;

public:
    Server(const std::string& port, int connectionBacklog, Config config, std::size_t numThreads);

    void handleClient(const Socket& client) const;

    void serve();
};
