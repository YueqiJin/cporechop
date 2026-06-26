/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */
#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "porechop/cli.hpp"
#include "porechop/io.hpp"
#include "porechop/types.hpp"

namespace porechop {

    struct PipelineResult {
        std::size_t read_count = 0;
        std::size_t trimmed_start_count = 0;
        std::size_t trimmed_end_count = 0;
        std::size_t start_bp_removed = 0;
        std::size_t end_bp_removed = 0;
        std::size_t split_count = 0;
        std::size_t discarded_middle_count = 0;
        std::map<std::string, std::size_t> barcode_counts;
    };

    PipelineResult run_pipeline(const PipelineOptions& options);

}  // namespace porechop
