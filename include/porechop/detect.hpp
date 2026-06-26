/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */
#pragma once

#include <string>
#include <vector>

#include "porechop/read.hpp"

namespace porechop {

    std::vector<AdapterSet> find_matching_adapter_sets(
        const std::vector<Read>& check_reads,
        const std::vector<AdapterSet>& search_adapters, int end_size,
        const ScoringScheme& scoring_scheme, double adapter_threshold,
        int threads = 1);

    /// Remove SQK-MAP006 Short from matching_sets when both 1D^2 parts are
    /// present and each has a score >= the SQK-MAP006 Short score.
    void fix_up_1d2_sets(std::vector<AdapterSet>& matching_sets);

    /// Choose barcode orientation (forward/reverse) based on aggregate scores.
    /// Throws std::runtime_error if no barcodes are found.
    std::string choose_barcoding_kit(
        const std::vector<AdapterSet>& adapter_sets);

}  // namespace porechop
