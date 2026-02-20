//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_THREAD_POOL_H_
#define LACE_THREAD_POOL_H_

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

namespace lace {

using Job = std::function<void()>;

class ThreadPool final {
    std::vector<std::jthread> m_threads = {};
    std::queue<Job> m_jobs = {};
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable m_done;
    uint32_t m_pending = 0;

public:
    ThreadPool(uint32_t count);

    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    void operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) noexcept = delete;
    void operator=(ThreadPool&&) noexcept = delete;

    /// Push the given |job| onto the back of this pool.
    void push(Job job);

    /// Wait for all jobs in this pool to complete.
    void wait();

    /// Returns the number of pending jobs in this pool.
    uint32_t get_pending() const { return m_pending; }

private:
    void worker(std::stop_token token);
    void requestStop();
};

} // namespace lace

#endif // LACE_THREAD_POOL_H_
