/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/pipeline.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "porechop/adapter_file.hpp"
#include "porechop/barcode.hpp"
#include "porechop/detect.hpp"
#include "porechop/full_barcode.hpp"
#include "porechop/middle.hpp"
#include "porechop/output.hpp"
#include "porechop/scoring.hpp"
#include "porechop/trim.hpp"

namespace porechop {

    // ---------------------------------------------------------------------------
    // Flatten a vector of AdapterSet into (name, sequence) pairs for middle
    // adapter detection.  Each non-empty start/end sequence is added.
    // ---------------------------------------------------------------------------
    static std::vector<AdapterSequence> flatten_for_middle(
        const std::vector<AdapterSet>& matching_sets) {
        std::vector<AdapterSequence> result;
        for (const auto& as : matching_sets) {
            if (!as.start_sequence.sequence.empty()) {
                result.push_back(
                    {as.start_sequence.name, as.start_sequence.sequence});
            }
            if (!as.end_sequence.sequence.empty() &&
                as.end_sequence.sequence != as.start_sequence.sequence) {
                result.push_back(
                    {as.end_sequence.name, as.end_sequence.sequence});
            }
        }
        return result;
    }

    // ---------------------------------------------------------------------------
    // Collect the set of start/end sequence names for good/bad side logic in
    // middle adapter detection.
    // ---------------------------------------------------------------------------
    static std::pair<std::set<std::string>, std::set<std::string>>
    collect_side_names(const std::vector<AdapterSet>& matching_sets) {
        std::set<std::string> start_names;
        std::set<std::string> end_names;
        for (const auto& as : matching_sets) {
            if (!as.start_sequence.name.empty())
                start_names.insert(as.start_sequence.name);
            if (!as.end_sequence.name.empty())
                end_names.insert(as.end_sequence.name);
        }
        return {std::move(start_names), std::move(end_names)};
    }

    // ---------------------------------------------------------------------------
    // run_pipeline
    // ---------------------------------------------------------------------------
    PipelineResult run_pipeline(const PipelineOptions& opts) {
        PipelineResult result;

        // ---- Step 1: Load reads
        // -----------------------------------------------
        std::vector<Read> reads = load_reads(opts.input);
        result.read_count = reads.size();

        if (reads.empty()) {
            return result;
        }

        // ---- Step 2: Determine check_reads subset
        // -----------------------------
        const std::size_t check_count = static_cast<std::size_t>(
            std::min(opts.check_reads, static_cast<int>(reads.size())));
        std::vector<Read> check_reads(
            reads.begin(),
            reads.begin() + static_cast<std::ptrdiff_t>(check_count));

        // ---- Step 3: Build adapter registry
        // -----------------------------------
        AdapterRegistry registry;
        if (!opts.no_default_adapters) {
            registry = AdapterRegistry::get_builtin();
        }
        if (!opts.adapter_file.empty()) {
            registry.add_file(opts.adapter_file);
        }
        std::vector<AdapterSet> search_adapters = registry.get_search();

        // ---- Step 4: Score all adapters against check_reads
        // ----------------------------------------
        std::vector<AdapterSet> scored_adapters = aggregate_adapter_scores(
            check_reads, search_adapters, opts.end_size,
            opts.scoring_scheme_vals, opts.threads);

        // ---- Step 5: Filter to matching adapter sets
        // ------------------------------
        std::vector<AdapterSet> matching_sets;
        for (const auto& adapter_set : scored_adapters) {
            if (adapter_set.best_start_or_end_score() >=
                opts.adapter_threshold) {
                matching_sets.push_back(adapter_set);
            }
        }

        // ---- Step 6: fix_up_1d2_sets
        // ------------------------------------------
        fix_up_1d2_sets(matching_sets);

        // ---- Step 7: Display adapter set results table
        // --------------------------------
        display_adapter_set_results(scored_adapters, matching_sets,
                                    opts.verbosity);

        // ---- Step 8: Barcode kit selection + full barcode construction
        // --------
        std::string barcode_orientation;
        bool check_barcodes = false;
        if (!opts.barcode_dir.empty()) {
            barcode_orientation = choose_barcoding_kit(matching_sets);
            check_barcodes = true;
        }
        // Full barcode adapter sets are always added (matches original
        // Porechop behaviour: porechop.py line 50 calls this unconditionally).
        matching_sets = add_full_barcode_adapter_sets(matching_sets);

        // ---- Step 9: End trimming
        // ---------------------------------------------
        display_trimming_header_and_adapters(matching_sets, opts.verbosity);
        find_adapters_at_read_ends(reads, matching_sets, opts.end_size,
                                   opts.extra_end_trim, opts.end_threshold,
                                   opts.scoring_scheme_vals, opts.min_trim_size,
                                   opts.threads, check_barcodes);

        // ---- Step 10: Middle adapter detection
        // ---------------------------------
        if (!opts.no_split) {
            auto middle_adapters = flatten_for_middle(matching_sets);
            auto [start_names, end_names] = collect_side_names(matching_sets);
            find_adapters_in_read_middles(
                reads, middle_adapters, opts.middle_threshold,
                opts.extra_middle_trim_good_side,
                opts.extra_middle_trim_bad_side, opts.scoring_scheme_vals,
                start_names, end_names, opts.threads);
        }

        // ---- Step 11: Barcode determination
        // ------------------------------------
        if (!opts.barcode_dir.empty()) {
            for (auto& read : reads) {
                determine_barcode(read, opts.barcode_threshold,
                                  opts.barcode_diff, opts.require_two_barcodes);
            }
        }

        // ---- Step 12: Compute result statistics
        // -------------------------------
        int total_start_bp = 0, total_end_bp = 0;
        for (const auto& read : reads) {
            if (read.start_trim_amount > 0) {
                result.trimmed_start_count++;
                total_start_bp += read.start_trim_amount;
            }
            if (read.end_trim_amount > 0) {
                result.trimmed_end_count++;
                total_end_bp += read.end_trim_amount;
            }
            // Match original Porechop: count reads with middle_adapter_positions
            // (the actual adapter hit region), not the broader trim region.
            if (!read.middle_adapter_positions.empty()) {
                result.split_count++;
            }
            if (opts.discard_middle && !read.middle_trim_positions.empty()) {
                result.discarded_middle_count++;
            }
            if (!opts.barcode_dir.empty()) {
                result.barcode_counts[read.barcode_call]++;
            }
        }
        result.start_bp_removed = static_cast<std::size_t>(total_start_bp);
        result.end_bp_removed = static_cast<std::size_t>(total_end_bp);

        // ---- Step 13: Output
        // --------------------------------------------------
        output_reads(reads, opts.format, opts.output, opts.barcode_dir,
                     opts.min_split_read_size, opts.discard_middle,
                     opts.untrimmed, opts.discard_unassigned, opts.verbosity);

        return result;
    }

}  // namespace porechop
