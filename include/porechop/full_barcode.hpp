#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <vector>

#include "porechop/types.hpp"

namespace porechop {

    /**
     * add_full_barcode_adapter_sets: appends full-length barcode adapter
     * sequences to the matching adapter set list when the necessary component
     * adapters are present.
     *
     * Native barcoding: when both SQK-NSK007 and Barcode N (reverse) are
     * present, a full native barcode adapter is appended.
     *
     * Rapid barcoding: when both Rapid and Barcode N (forward) are present:
     *   - if RBK004_upstream is also present -> new rapid barcode (SQK-RBK004)
     *   - elif SQK-NSK007 is present        -> old rapid barcode (SQK-RBK001)
     *
     * The original vector is preserved unchanged; full barcode adapters are
     * appended at the end.
     */
    std::vector<AdapterSet> add_full_barcode_adapter_sets(
        const std::vector<AdapterSet>& matching_sets);

}  // namespace porechop
