/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */
#pragma once
#include <cstddef>
#include <functional>
namespace porechop {
void parallel_for(std::size_t task_count, int threads,
                  const std::function<void(std::size_t)>& task);
}
