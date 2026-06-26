#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <vector>

#include "porechop/read.hpp"
#include "porechop/types.hpp"

namespace porechop {

/// Align one read's start/end windows against one adapter set and update the
/// adapter's best scores.
void score_adapter_set(const Read& read, AdapterSet& adapter_set, int end_size,
                       const ScoringScheme& scoring_scheme = {});

/// Score all reads against all adapter sets using per-thread-local adapter
/// copies and max reduction.
[[nodiscard]] std::vector<AdapterSet> aggregate_adapter_scores(
    const std::vector<Read>& reads, const std::vector<AdapterSet>& adapter_sets,
    int end_size, const ScoringScheme& scoring_scheme = {}, int threads = 1);

}  // namespace porechop
