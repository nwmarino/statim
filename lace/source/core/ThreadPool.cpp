//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/ThreadPool.h"

using namespace lace;

ThreadPool::ThreadPool(uint32_t count) {
    m_threads.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        m_threads.emplace_back([this](std::stop_token token) {
            worker(token);
        });
    }
}

ThreadPool::~ThreadPool() {
    requestStop();
}

void ThreadPool::push(Job job) {
    {
        std::lock_guard guard(m_mutex);
        m_jobs.push(std::move(job));
        ++m_pending;
    }

    m_cv.notify_one();
}

void ThreadPool::wait() {
    std::unique_lock lock(m_mutex);
    m_done.wait(lock, [&] {
        return m_pending == 0;
    });
}

void ThreadPool::worker(std::stop_token token) {
    while (!token.stop_requested()) {
        Job job;

        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [&] {
                return token.stop_requested() || !m_jobs.empty();
            });

            if (token.stop_requested())
                return;

            job = std::move(m_jobs.front());
            m_jobs.pop();
        }

        job();

        {
            std::lock_guard lock(m_mutex);
            --m_pending;
            
            if (m_pending == 0)
                m_done.notify_all();
        }
    }
}

void ThreadPool::requestStop() {
    for (std::jthread& thr : m_threads)
        thr.request_stop();

    m_cv.notify_all();
}
