#include "ThreadPool.hpp"
#include "Socket.hpp"
#include <cstddef>
#include <mutex>
#include <utility>

ThreadPool::ThreadPool(std::size_t nthreads, std::function<void(const Socket&)> handler) : m_handler { handler }
{
    for (auto i { 0uz }; i < nthreads; ++i)
    {
        m_workers.emplace_back([this]() {
            while (true)
            {
                Socket client { Socket::invalidFD };
                {
                    std::unique_lock lock { m_queueMutex };
                    m_queueCV.wait(lock, [this]() { return m_stop || !m_tasks.empty(); });

                    if (m_stop && m_tasks.empty())
                        return;

                    client = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                m_handler(client);
            }
        });
    }
}

void ThreadPool::enqueue(Socket client)
{
    {
        std::scoped_lock lock { m_queueMutex };
        m_tasks.push(std::move(client));
    }

    m_queueCV.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        std::scoped_lock lock { m_queueMutex };
        m_stop = true;
    }

    m_queueCV.notify_all();
    for (auto& worker : m_workers)
        worker.join();
}
