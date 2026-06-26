#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>

#include "porechop/types.hpp"

namespace porechop {

    /// Align an adapter sequence against a read using seqan3 semi-global
    /// alignment. Returns a fully-populated AdapterAlignment including
    /// identity metrics computed via column-walking.
    AdapterAlignment align_adapter(std::string_view read,
                                   std::string_view adapter,
                                   const ScoringScheme& scoring_scheme = {});

    /// Align native seqan3 dna5 vectors without a string conversion step.
    AdapterAlignment align_adapter(const seqan3::dna5_vector& read,
                                   const seqan3::dna5_vector& adapter,
                                   const ScoringScheme& scoring_scheme = {});

}  // namespace porechop
