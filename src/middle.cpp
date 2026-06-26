/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/middle.hpp"

#include <algorithm>
#include <string>
#include <string_view>

#include "porechop/threads.hpp"

// Forward declaration — avoids seqan3 transitive dependency for MSVC
// compatibility. The linker resolves this symbol from src/align.cpp.
namespace porechop {
    AdapterAlignment align_adapter(std::string_view read,
                                   std::string_view adapter,
                                   const ScoringScheme& scoring_scheme = {});
}

namespace porechop {

    void Read::find_middle_adapters(
        const std::vector<AdapterSequence>& adapters, double middle_threshold,
        int extra_middle_trim_good_side, int extra_middle_trim_bad_side,
        const ScoringScheme& scoring_scheme,
        const std::set<std::string>& start_sequence_names,
        const std::set<std::string>& end_sequence_names) {
        std::string masked_seq = get_seq_with_start_end_adapters_trimmed();
        if (masked_seq.empty()) return;

        for (const auto& [adapter_name, adapter_seq] : adapters) {
            // Repeatedly align until no more strong hits
            while (true) {
                const AdapterAlignment aln =
                    align_adapter(masked_seq, adapter_seq, scoring_scheme);

                if (aln.full_adapter_percent_id < middle_threshold) {
                    break;
                }

                if (aln.read_begin_pos < 0 ||
                    aln.read_end_pos <= aln.read_begin_pos) {
                    break;
                }

                const int read_start = aln.read_begin_pos;
                const int read_end = aln.read_end_pos;

                // Mask the hit region so it won't be rediscovered
                const auto mask_len =
                    static_cast<std::size_t>(read_end - read_start);
                const auto mask_start = static_cast<std::size_t>(read_start);
                masked_seq.replace(mask_start, mask_len,
                                   std::string(mask_len, '-'));

                // Record adapter positions and trim positions
                for (int i = read_start; i < read_end; ++i) {
                    middle_adapter_positions.insert(i);
                }

                // Build hit string
                middle_hit_str +=
                    "  " + adapter_name +
                    " (read coords: " + std::to_string(read_start) + "-" +
                    std::to_string(read_end) + ", identity: " +
                    std::to_string(aln.full_adapter_percent_id) + "%)\n";

                // Determine trim range based on adapter side
                int trim_start = read_start - extra_middle_trim_good_side;
                if (start_sequence_names.count(adapter_name)) {
                    trim_start = read_start - extra_middle_trim_bad_side;
                }

                int trim_end = read_end + extra_middle_trim_good_side;
                if (end_sequence_names.count(adapter_name)) {
                    trim_end = read_end + extra_middle_trim_bad_side;
                }

                // Clip trim range to valid bounds
                const int mask_len_int = static_cast<int>(masked_seq.size());
                trim_start = std::max(0, trim_start);
                trim_end = std::min(trim_end, mask_len_int);

                for (int i = trim_start; i < trim_end; ++i) {
                    middle_trim_positions.insert(i);
                }
            }
        }
    }

    void find_adapters_in_read_middles(
        std::vector<Read>& reads, const std::vector<AdapterSequence>& adapters,
        double middle_threshold, int extra_middle_trim_good_side,
        int extra_middle_trim_bad_side, const ScoringScheme& scoring_scheme,
        const std::set<std::string>& start_sequence_names,
        const std::set<std::string>& end_sequence_names, int threads) {
        parallel_for(reads.size(), threads, [&](std::size_t i) {
            reads[i].find_middle_adapters(
                adapters, middle_threshold, extra_middle_trim_good_side,
                extra_middle_trim_bad_side, scoring_scheme,
                start_sequence_names, end_sequence_names);
        });
    }

}  // namespace porechop
