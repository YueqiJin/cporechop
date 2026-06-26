/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */
#pragma once
#include "porechop/read.hpp"
namespace porechop {
    void find_adapters_in_read_middles(
        std::vector<Read>& reads, const std::vector<AdapterSequence>& adapters,
        double middle_threshold, int extra_trim_good_side,
        int extra_trim_bad_side, const ScoringScheme& scoring_scheme,
        const std::set<std::string>& start_sequence_names,
        const std::set<std::string>& end_sequence_names, int threads = 1);
}
