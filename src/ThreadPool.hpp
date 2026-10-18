#pragma once

#include "Socket.hpp"
#include <condition_variable>
#include <cstddef>
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
    void (*m_handler)(const Socket&);
    std::vector<std::thread> m_workers;

public:
    ThreadPool(std::size_t nthreads, void (*handler)(const Socket&));

    void enqueue(Socket);

    ~ThreadPool();
};
