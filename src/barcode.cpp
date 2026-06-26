/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/barcode.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace porechop {
    namespace {

        std::vector<BarcodeScore> sorted_scores(
            const std::unordered_map<std::string, double>& scores) {
            std::vector<BarcodeScore> sorted(scores.begin(), scores.end());
            std::sort(sorted.begin(), sorted.end(),
                      [](const BarcodeScore& lhs, const BarcodeScore& rhs) {
                          if (lhs.second != rhs.second) {
                              return lhs.second > rhs.second;
                          }
                          return lhs.first < rhs.first;
                      });
            return sorted;
        }

        BarcodeScore score_or_none(const std::vector<BarcodeScore>& scores,
                                   std::size_t index) {
            if (index < scores.size()) {
                return scores[index];
            }
            return {"none", 0.0};
        }

        std::vector<BarcodeScore> combine_start_end_scores(
            const std::vector<BarcodeScore>& start_scores,
            const std::vector<BarcodeScore>& end_scores) {
            std::vector<BarcodeScore> all_scores;
            all_scores.reserve(start_scores.size() + end_scores.size());
            all_scores.insert(all_scores.end(), start_scores.begin(),
                              start_scores.end());
            all_scores.insert(all_scores.end(), end_scores.begin(),
                              end_scores.end());

            std::sort(all_scores.begin(), all_scores.end(),
                      [](const BarcodeScore& lhs, const BarcodeScore& rhs) {
                          if (lhs.second != rhs.second) {
                              return lhs.second > rhs.second;
                          }
                          return lhs.first < rhs.first;
                      });

            std::vector<BarcodeScore> combined;
            combined.reserve(all_scores.size());
            for (const auto& score : all_scores) {
                const auto already_included =
                    std::find_if(combined.begin(), combined.end(),
                                 [&score](const BarcodeScore& kept) {
                                     return kept.first == score.first;
                                 });
                if (already_included == combined.end()) {
                    combined.push_back(score);
                }
            }
            return combined;
        }

        bool passes_threshold_and_diff(const BarcodeScore& best,
                                       const BarcodeScore& second,
                                       double threshold, double diff) {
            return best.second >= threshold &&
                   best.second >= second.second + diff;
        }

    }  // namespace

    void determine_barcode(Read& read, double barcode_threshold,
                           double barcode_diff, bool require_two_barcodes) {
        const std::vector<BarcodeScore> start_barcode_scores =
            sorted_scores(read.start_barcode_scores);
        const std::vector<BarcodeScore> end_barcode_scores =
            sorted_scores(read.end_barcode_scores);

        read.best_start_barcode = score_or_none(start_barcode_scores, 0);
        read.second_best_start_barcode = score_or_none(start_barcode_scores, 1);
        read.second_best_start = read.second_best_start_barcode;

        read.best_end_barcode = score_or_none(end_barcode_scores, 0);
        read.second_best_end_barcode = score_or_none(end_barcode_scores, 1);
        read.second_best_end = read.second_best_end_barcode;

        read.barcode_call = "none";

        if (require_two_barcodes) {
            const bool start_good = passes_threshold_and_diff(
                read.best_start_barcode, read.second_best_start_barcode,
                barcode_threshold, barcode_diff);
            const bool end_good = passes_threshold_and_diff(
                read.best_end_barcode, read.second_best_end_barcode,
                barcode_threshold, barcode_diff);
            if (start_good && end_good &&
                read.best_start_barcode.first == read.best_end_barcode.first) {
                read.barcode_call = read.best_start_barcode.first;
            }
        } else {
            const std::vector<BarcodeScore> combined_scores =
                combine_start_end_scores(start_barcode_scores,
                                         end_barcode_scores);
            const BarcodeScore best_overall = score_or_none(combined_scores, 0);
            const BarcodeScore second_best_overall =
                score_or_none(combined_scores, 1);

            if (passes_threshold_and_diff(best_overall, second_best_overall,
                                          barcode_threshold, barcode_diff)) {
                read.barcode_call = best_overall.first;
            }
        }

        if (read.albacore_barcode_call &&
            read.barcode_call != *read.albacore_barcode_call) {
            read.barcode_call = "none";
        }
    }

}  // namespace porechop
