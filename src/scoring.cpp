/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/scoring.hpp"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <vector>

// Forward declaration — avoids seqan3 transitive dependency for MSVC
// compatibility. The linker resolves this symbol from src/align.cpp.
namespace porechop {
    AdapterAlignment align_adapter(std::string_view read,
                                   std::string_view adapter,
                                   const ScoringScheme& scoring_scheme = {});
}

namespace porechop {
    namespace {

        std::size_t read_end_window_size(const Read& read, int end_size) {
            return std::min(read.seq.size(),
                            static_cast<std::size_t>(std::max(0, end_size)));
        }

        void merge_adapter_scores(std::vector<AdapterSet>& destination,
                                  const std::vector<AdapterSet>& source) {
            for (std::size_t i = 0; i < destination.size(); ++i) {
                destination[i].best_start_score =
                    std::max(destination[i].best_start_score,
                             source[i].best_start_score);
                destination[i].best_end_score = std::max(
                    destination[i].best_end_score, source[i].best_end_score);
            }
        }

        double score_from_alignment(const AdapterAlignment& alignment,
                                    const AdapterSequence& adapter,
                                    const ScoringScheme& scheme) {
            if (alignment.full_adapter_percent_id > 0.0) {
                return alignment.full_adapter_percent_id;
            }
            if (alignment.raw_score <= 0 || adapter.sequence.empty() ||
                scheme.match <= 0) {
                return 0.0;
            }

            const double perfect_score =
                static_cast<double>(adapter.sequence.size() *
                                    static_cast<std::size_t>(scheme.match));
            return std::min(100.0,
                            100.0 * static_cast<double>(alignment.raw_score) /
                                perfect_score);
        }

    }  // namespace

    void score_adapter_set(const Read& read, AdapterSet& adapter_set,
                           int end_size, const ScoringScheme& scoring_scheme) {
        const std::size_t window_size = read_end_window_size(read, end_size);

        if (!adapter_set.start_sequence.sequence.empty()) {
            const AdapterAlignment alignment = align_adapter(
                std::string_view{read.seq.data(), window_size},
                adapter_set.start_sequence.sequence, scoring_scheme);
            adapter_set.best_start_score = std::max(
                adapter_set.best_start_score,
                score_from_alignment(alignment, adapter_set.start_sequence,
                                     scoring_scheme));
        }

        if (!adapter_set.end_sequence.sequence.empty()) {
            const AdapterAlignment alignment = align_adapter(
                std::string_view{
                    read.seq.data() + read.seq.size() - window_size,
                    window_size},
                adapter_set.end_sequence.sequence, scoring_scheme);
            adapter_set.best_end_score = std::max(
                adapter_set.best_end_score,
                score_from_alignment(alignment, adapter_set.end_sequence,
                                     scoring_scheme));
        }
    }

    std::vector<AdapterSet> aggregate_adapter_scores(
        const std::vector<Read>& reads,
        const std::vector<AdapterSet>& adapter_sets, int end_size,
        const ScoringScheme& scoring_scheme, int threads) {
        if (threads < 1) {
            throw std::runtime_error("thread count must be at least 1");
        }

        std::vector<AdapterSet> merged = adapter_sets;
        if (reads.empty() || adapter_sets.empty()) {
            return merged;
        }

        if (threads == 1) {
            for (std::size_t read_index = 0; read_index < reads.size();
                 ++read_index) {
                for (std::size_t adapter_index = 0;
                     adapter_index < adapter_sets.size(); ++adapter_index) {
                    score_adapter_set(reads[read_index], merged[adapter_index],
                                      end_size, scoring_scheme);
                }
            }
            return merged;
        }

        const std::size_t worker_count = std::min<std::size_t>(
            static_cast<std::size_t>(threads), reads.size());
        std::vector<std::vector<AdapterSet>> local_scores(worker_count,
                                                          adapter_sets);
        std::vector<std::exception_ptr> exceptions(worker_count);
        std::vector<std::thread> workers;
        workers.reserve(worker_count);

        for (std::size_t worker_index = 0; worker_index < worker_count;
             ++worker_index) {
            workers.emplace_back([&, worker_index] {
                try {
                    for (std::size_t read_index = worker_index;
                         read_index < reads.size();
                         read_index += worker_count) {
                        for (std::size_t adapter_index = 0;
                             adapter_index < adapter_sets.size();
                             ++adapter_index) {
                            score_adapter_set(
                                reads[read_index],
                                local_scores[worker_index][adapter_index],
                                end_size, scoring_scheme);
                        }
                    }
                } catch (...) {
                    exceptions[worker_index] = std::current_exception();
                }
            });
        }

        for (auto& worker : workers) {
            worker.join();
        }
        for (const auto& exception : exceptions) {
            if (exception) {
                std::rethrow_exception(exception);
            }
        }

        for (const auto& local_score : local_scores) {
            merge_adapter_scores(merged, local_score);
        }
        return merged;
    }

}  // namespace porechop
