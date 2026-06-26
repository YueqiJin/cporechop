/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/detect.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

#include "porechop/scoring.hpp"

namespace porechop {

    std::vector<AdapterSet> find_matching_adapter_sets(
        const std::vector<Read>& reads, const std::vector<AdapterSet>& adapters,
        int end_size, const ScoringScheme& scheme, double threshold,
        int threads) {
        const std::vector<AdapterSet> scored_adapters =
            aggregate_adapter_scores(reads, adapters, end_size, scheme,
                                     threads);

        std::vector<AdapterSet> matching_adapters;
        for (const auto& adapter_set : scored_adapters) {
            if (adapter_set.best_start_or_end_score() >= threshold) {
                matching_adapters.push_back(adapter_set);
            }
        }

        return matching_adapters;
    }

    void fix_up_1d2_sets(std::vector<AdapterSet>& matching_sets) {
        bool has_part1 = false, has_part2 = false, has_sqk_short = false;
        double part1_score = 0.0, part2_score = 0.0, sqk_score = 0.0;

        for (const auto& as : matching_sets) {
            if (as.name == "1D^2 part 1") {
                has_part1 = true;
                part1_score = as.best_start_or_end_score();
            } else if (as.name == "1D^2 part 2") {
                has_part2 = true;
                part2_score = as.best_start_or_end_score();
            } else if (as.name == "SQK-MAP006 short" ||
                       as.name == "SQK-MAP006 Short") {
                has_sqk_short = true;
                sqk_score = as.best_start_or_end_score();
            }
        }

        if (has_part1 && has_part2 && has_sqk_short &&
            part1_score >= sqk_score && part2_score >= sqk_score) {
            matching_sets.erase(
                std::remove_if(matching_sets.begin(), matching_sets.end(),
                               [](const AdapterSet& as) {
                                   return as.name == "SQK-MAP006 short" ||
                                          as.name == "SQK-MAP006 Short";
                               }),
                matching_sets.end());
        }
    }

    std::string choose_barcoding_kit(
        const std::vector<AdapterSet>& adapter_sets) {
        double forward_start_or_end = 0.0, reverse_start_or_end = 0.0;
        double forward_start_and_end = 0.0, reverse_start_and_end = 0.0;

        for (const auto& adapter_set : adapter_sets) {
            if (!adapter_set.is_barcode()) continue;

            std::string lower_name = adapter_set.name;
            std::transform(
                lower_name.begin(), lower_name.end(), lower_name.begin(),
                [](char c) { return static_cast<char>(std::tolower(c)); });

            if (lower_name.find("(forward)") != std::string::npos) {
                forward_start_or_end += adapter_set.best_start_or_end_score();
                forward_start_and_end += adapter_set.best_start_score;
                forward_start_and_end += adapter_set.best_end_score;
            } else if (lower_name.find("(reverse)") != std::string::npos) {
                reverse_start_or_end += adapter_set.best_start_or_end_score();
                reverse_start_and_end += adapter_set.best_start_score;
                reverse_start_and_end += adapter_set.best_end_score;
            }
        }

        if (forward_start_or_end == 0.0 && reverse_start_or_end == 0.0) {
            throw std::runtime_error(
                "Error: no barcodes were found, so Porechop cannot perform "
                "barcode demultiplexing");
        }

        std::string orientation;
        // Primary tie-breaker: best start OR end score
        if (forward_start_or_end > reverse_start_or_end) {
            orientation = "forward";
        } else if (reverse_start_or_end > forward_start_or_end) {
            orientation = "reverse";
        }
        // Secondary tie-breaker: sum of both start AND end scores
        else if (forward_start_and_end > reverse_start_and_end) {
            orientation = "forward";
        } else if (reverse_start_and_end > forward_start_and_end) {
            orientation = "reverse";
        }

        if (orientation.empty()) {
            throw std::runtime_error(
                "Error: Porechop could not determine barcode orientation");
        }

        return orientation;
    }

}  // namespace porechop
