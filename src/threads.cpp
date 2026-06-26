/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/threads.hpp"

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <exception>
#include <stdexcept>
#include <vector>
namespace porechop {
    void parallel_for(std::size_t task_count, int threads,
                      const std::function<void(std::size_t)>& task) {
        if (threads < 1)
            throw std::runtime_error("thread count must be at least 1");
        if (threads == 1 || task_count < 2) {
            for (std::size_t i = 0; i < task_count; ++i) task(i);
            return;
        }
        boost::asio::thread_pool pool(static_cast<std::size_t>(threads));
        std::vector<std::exception_ptr> exceptions(task_count);
        for (std::size_t i = 0; i < task_count; ++i)
            boost::asio::post(pool, [&, i] {
                try {
                    task(i);
                } catch (...) {
                    exceptions[i] = std::current_exception();
                }
            });
        pool.join();
        for (const auto& e : exceptions)
            if (e) std::rethrow_exception(e);
    }
}  // namespace porechop
