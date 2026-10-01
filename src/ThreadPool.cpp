#include "ThreadPool.h"
#include "Socket.h"
#include <cstddef>
#include <mutex>

ThreadPool::ThreadPool(std::function<void(const Socket&)> handler, std::size_t numThreads) : m_handler { handler }
{
    for (std::size_t i { 0 }; i < numThreads; ++i)
        m_workers.emplace_back([this] {
            while (true)
            {
                Socket client { Socket::getInvalidFD() };
                {
                    std::unique_lock lock { m_queueMutex };
                    m_cv.wait(lock, [this]() { return m_stop || !m_tasks.empty(); });

                    if (m_stop && m_tasks.empty())
                        return;

                    client = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                m_handler(client);
            }
        });
}

ThreadPool::~ThreadPool()
{
    {
        std::scoped_lock lock { m_queueMutex };
        m_stop = true;
    }

    m_cv.notify_all();

    for (auto& worker : m_workers)
        worker.join();
}

void ThreadPool::enqueue(Socket client)
{
    {
        std::scoped_lock lock(m_queueMutex);
        m_tasks.push(std::move(client));
    }
    m_cv.notify_one();
}
