/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/align.hpp"

#include <algorithm>
#include <ranges>
#include <seqan3/alignment/configuration/all.hpp>
#include <seqan3/alignment/pairwise/align_pairwise.hpp>
#include <seqan3/alignment/scoring/nucleotide_scoring_scheme.hpp>
#include <seqan3/alphabet/views/all.hpp>
#include <tuple>

#include "porechop/identity.hpp"

namespace porechop {

    namespace {

        seqan3::dna5_vector to_dna5_vector(std::string_view sequence) {
            auto view = sequence | seqan3::views::char_to<seqan3::dna5>;
            return seqan3::dna5_vector{std::ranges::begin(view),
                                       std::ranges::end(view)};
        }

        auto make_alignment_config(const ScoringScheme& scheme) {
            using namespace seqan3::align_cfg;

            return method_global{free_end_gaps_sequence1_leading{true},
                                 free_end_gaps_sequence2_leading{true},
                                 free_end_gaps_sequence1_trailing{true},
                                 free_end_gaps_sequence2_trailing{true}} |
                   scoring_scheme{seqan3::nucleotide_scoring_scheme{
                       seqan3::match_score{scheme.match},
                       seqan3::mismatch_score{scheme.mismatch}}} |
                   gap_cost_affine{open_score{scheme.gap_open},
                                   extension_score{scheme.gap_extend}} |
                   output_score{} | output_begin_position{} |
                   output_end_position{} | output_alignment{};
        }

        AdapterAlignment sentinel_alignment() { return AdapterAlignment{}; }

    }  // namespace

    AdapterAlignment align_adapter(std::string_view read,
                                   std::string_view adapter,
                                   const ScoringScheme& scoring_scheme) {
        if (read.empty() || adapter.empty()) {
            return sentinel_alignment();
        }

        return align_adapter(to_dna5_vector(read), to_dna5_vector(adapter),
                             scoring_scheme);
    }

    AdapterAlignment align_adapter(const seqan3::dna5_vector& read,
                                   const seqan3::dna5_vector& adapter,
                                   const ScoringScheme& scoring_scheme) {
        if (read.empty() || adapter.empty()) {
            return sentinel_alignment();
        }

        auto configuration = make_alignment_config(scoring_scheme);
        auto result_range =
            seqan3::align_pairwise(std::tie(read, adapter), configuration);
        auto result_it = std::ranges::begin(result_range);

        if (result_it == std::ranges::end(result_range)) {
            return sentinel_alignment();
        }

        const auto& result = *result_it;

        // seqan3's output_alignment gives only the compact overlapping core.
        // To match original Porechop's seqan2 globalAlignment (which returns
        // gapped strings spanning the full read and adapter), we reconstruct
        // the full alignment rows by adding the unaligned prefix/suffix parts.
        auto&& [gapped_read, gapped_adapter] = result.alignment();

        auto core_read_chars = gapped_read | seqan3::views::to_char;
        auto core_adapter_chars = gapped_adapter | seqan3::views::to_char;
        std::string core_read_aln(core_read_chars.begin(), core_read_chars.end());
        std::string core_adapter_aln(core_adapter_chars.begin(),
                                     core_adapter_chars.end());

        auto read_str_chars = read | seqan3::views::to_char;
        auto adapter_str_chars = adapter | seqan3::views::to_char;
        std::string read_str(read_str_chars.begin(), read_str_chars.end());
        std::string adapter_str(adapter_str_chars.begin(), adapter_str_chars.end());

        const int read_begin =
            static_cast<int>(result.sequence1_begin_position());
        const int read_end = static_cast<int>(result.sequence1_end_position());
        const int adapter_begin =
            static_cast<int>(result.sequence2_begin_position());
        const int adapter_end =
            static_cast<int>(result.sequence2_end_position());

        std::string full_read_aln;
        std::string full_adapter_aln;
        full_read_aln.reserve(read.size() + adapter.size());
        full_adapter_aln.reserve(read.size() + adapter.size());

        // Unaligned read prefix
        full_read_aln.append(read_str.substr(0, read_begin));
        full_adapter_aln.append(static_cast<std::size_t>(read_begin), '-');

        // Unaligned adapter prefix
        full_read_aln.append(static_cast<std::size_t>(adapter_begin), '-');
        full_adapter_aln.append(adapter_str.substr(0, adapter_begin));

        // Aligned core
        full_read_aln += core_read_aln;
        full_adapter_aln += core_adapter_aln;

        // Unaligned read suffix
        full_read_aln.append(read_str.substr(read_end));
        full_adapter_aln.append(read_str.size() - read_end, '-');

        // Unaligned adapter suffix
        full_read_aln.append(adapter_str.size() - adapter_end, '-');
        full_adapter_aln.append(adapter_str.substr(adapter_end));

        AdapterAlignment alignment = compute_identity_from_alignment_strings(
            full_read_aln, full_adapter_aln,
            static_cast<int>(result.score()));

        // compute_identity_from_alignment_strings returns column-walked
        // positions matching original Porechop: begin = index of first aligned
        // base, end = index of last aligned base. The rest of the pipeline
        // expects half-open intervals, so add one to end positions.
        alignment.read_end_pos = alignment.read_end_pos + 1;
        alignment.adapter_end_pos = alignment.adapter_end_pos + 1;

        return alignment;
    }

}  // namespace porechop
