/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/trim.hpp"

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

    void Read::find_start_trim(const std::vector<AdapterSet>& adapters,
                               int end_size, int extra_trim_size,
                               double end_threshold,
                               const ScoringScheme& scoring_scheme,
                               int min_trim_size, bool check_barcodes,
                               const std::string& /*barcode_dir*/) {
        if (seq.empty()) return;

        const std::string read_seq_start =
            seq.substr(0, static_cast<std::size_t>(std::min(
                              end_size, static_cast<int>(seq.size()))));
        const int actual_end_size = static_cast<int>(read_seq_start.size());

        for (const auto& adapter : adapters) {
            if (adapter.start_sequence.sequence.empty()) continue;

            const AdapterAlignment aln =
                align_adapter(read_seq_start, adapter.start_sequence.sequence,
                              scoring_scheme);

            // Guard: partial_score > end_threshold, read_end != end_size,
            // read_end - read_start >= min_trim_size
            if (aln.aligned_region_percent_id > end_threshold &&
                aln.read_end_pos != actual_end_size &&
                (aln.read_end_pos - aln.read_begin_pos) >= min_trim_size) {
                const int trim_amount = aln.read_end_pos + extra_trim_size;
                start_trim_amount = std::max(start_trim_amount, trim_amount);

                start_adapter_alignments.emplace_back(
                    adapter, aln.full_adapter_percent_id,
                    aln.aligned_region_percent_id, aln.read_begin_pos,
                    aln.read_end_pos);
            }

            if (check_barcodes && adapter.is_barcode() &&
                !adapter.get_barcode_name().empty()) {
                start_barcode_scores[adapter.get_barcode_name()] =
                    aln.full_adapter_percent_id;
            }
        }
    }

    void Read::find_end_trim(const std::vector<AdapterSet>& adapters,
                             int end_size, int extra_trim_size,
                             double end_threshold,
                             const ScoringScheme& scoring_scheme,
                             int min_trim_size, bool check_barcodes,
                             const std::string& /*barcode_dir*/) {
        if (seq.empty()) return;

        const int seq_len = static_cast<int>(seq.size());
        const int slice_len = std::min(end_size, seq_len);
        const std::string read_seq_end =
            seq.substr(static_cast<std::size_t>(seq_len - slice_len));

        for (const auto& adapter : adapters) {
            if (adapter.end_sequence.sequence.empty()) continue;

            const AdapterAlignment aln = align_adapter(
                read_seq_end, adapter.end_sequence.sequence, scoring_scheme);

            // Guard: partial_score > end_threshold, read_start != 0, read_end -
            // read_start >= min_trim_size
            if (aln.aligned_region_percent_id > end_threshold &&
                aln.read_begin_pos != 0 &&
                (aln.read_end_pos - aln.read_begin_pos) >= min_trim_size) {
                const int trim_amount =
                    (slice_len - aln.read_begin_pos) + extra_trim_size;
                end_trim_amount = std::max(end_trim_amount, trim_amount);

                end_adapter_alignments.emplace_back(
                    adapter, aln.full_adapter_percent_id,
                    aln.aligned_region_percent_id, aln.read_begin_pos,
                    aln.read_end_pos);
            }

            if (check_barcodes && adapter.is_barcode() &&
                !adapter.get_barcode_name().empty()) {
                end_barcode_scores[adapter.get_barcode_name()] =
                    aln.full_adapter_percent_id;
            }
        }
    }

    void find_adapters_at_read_ends(
        std::vector<Read>& reads, const std::vector<AdapterSet>& sets,
        int end_size, int extra, double threshold, const ScoringScheme& scheme,
        int min_trim, int threads, bool check_barcodes,
        double /*barcode_threshold*/, double /*barcode_diff*/,
        bool /*require_two_barcodes*/, bool /*forward_or_reverse*/) {
        const std::string barcode_dir_str;
        parallel_for(reads.size(), threads, [&](std::size_t i) {
            reads[i].find_start_trim(sets, end_size, extra, threshold, scheme,
                                     min_trim, check_barcodes, barcode_dir_str);
            reads[i].find_end_trim(sets, end_size, extra, threshold, scheme,
                                   min_trim, check_barcodes, barcode_dir_str);
        });
    }

}  // namespace porechop
