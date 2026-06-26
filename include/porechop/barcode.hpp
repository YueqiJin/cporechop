#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/read.hpp"

namespace porechop {

    /// Determine and store the barcode call for a read from its start/end
    /// barcode scores.
    void determine_barcode(Read& read, double barcode_threshold,
                           double barcode_diff, bool require_two_barcodes);

}  // namespace porechop
