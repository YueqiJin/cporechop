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
void find_adapters_at_read_ends(
    std::vector<Read>& reads, const std::vector<AdapterSet>& matching_sets,
    int end_size, int extra_trim_size, double end_threshold,
    const ScoringScheme& scoring_scheme, int min_trim_size, int threads = 1,
    bool check_barcodes = false, double barcode_threshold = 75.0,
    double barcode_diff = 5.0, bool require_two_barcodes = false,
    bool forward_or_reverse_barcodes = true);
}
