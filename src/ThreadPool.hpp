#pragma once

#include "Socket.hpp"
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
private:
    std::queue<Socket> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    bool m_stop { false };
    std::function<void(const Socket&)> m_handler;
    std::vector<std::thread> m_workers;

public:
    ThreadPool(std::size_t nthreads, std::function<void(const Socket&)>);

    void enqueue(Socket);

    ~ThreadPool();
};
