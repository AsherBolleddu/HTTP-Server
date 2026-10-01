#pragma once

#include "Socket.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
private:
    std::vector<std::thread> m_workers;
    std::queue<Socket> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    bool m_stop { false };
    std::function<void(const Socket&)> m_handler;

public:
    ThreadPool(std::function<void(const Socket&)> m_handler, std::size_t numThreads);
    void enqueue(Socket client);
    ~ThreadPool();
};
