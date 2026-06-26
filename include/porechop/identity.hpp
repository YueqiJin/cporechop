#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <string_view>

#include "porechop/types.hpp"

namespace porechop {

    /// Compute Porechop identity metrics from two already-gapped alignment
    /// rows. Gaps must be represented with '-'. The rows are walked column by
    /// column to match the original ScoredAlignment implementation.
    AdapterAlignment compute_identity_from_alignment_strings(
        std::string_view read_alignment, std::string_view adapter_alignment,
        int raw_score = 0);


}  // namespace porechop
