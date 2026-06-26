#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "porechop/types.hpp"

namespace porechop {

    using ReadPart = std::pair<std::string, std::string>;
    using AdapterAlignmentRecord =
        std::tuple<AdapterSet, double, double, int, int>;
    using BarcodeScore = std::pair<std::string, double>;

    struct Read {
        std::string name;
        std::string seq;
        std::string quals;

        bool rna = false;

        int start_trim_amount = 0;
        int end_trim_amount = 0;
        std::vector<AdapterAlignmentRecord> start_adapter_alignments;
        std::vector<AdapterAlignmentRecord> end_adapter_alignments;

        std::set<int> middle_adapter_positions;
        std::set<int> middle_trim_positions;
        std::string middle_hit_str;

        std::unordered_map<std::string, double> start_barcode_scores;
        std::unordered_map<std::string, double> end_barcode_scores;

        BarcodeScore best_start_barcode{"none", 0.0};
        BarcodeScore best_end_barcode{"none", 0.0};
        BarcodeScore second_best_start{"none", 0.0};
        BarcodeScore second_best_end{"none", 0.0};
        BarcodeScore second_best_start_barcode{"none", 0.0};
        BarcodeScore second_best_end_barcode{"none", 0.0};
        std::string barcode_call = "none";

        std::optional<std::string> albacore_barcode_call;

        Read() = default;

        Read(std::string read_name, std::string read_seq,
             std::string read_quals = {})
            : name(std::move(read_name)),
              seq(std::move(read_seq)),
              quals(std::move(read_quals)) {
            std::transform(seq.begin(), seq.end(), seq.begin(),
                           [](unsigned char c) {
                               return static_cast<char>(std::toupper(c));
                           });

            const auto u_count = static_cast<std::size_t>(
                std::count(seq.begin(), seq.end(), 'U'));
            const auto t_count = static_cast<std::size_t>(
                std::count(seq.begin(), seq.end(), 'T'));
            rna = u_count > t_count;
            if (rna) {
                std::replace(seq.begin(), seq.end(), 'U', 'T');
            }

            if (quals.size() < seq.size()) {
                quals += std::string(seq.size() - quals.size(), '+');
            }
        }

        [[nodiscard]] std::string get_seq_with_start_end_adapters_trimmed()
            const {
            const auto [start_pos, end_pos] = trimmed_bounds(seq.size());
            return seq.substr(start_pos, end_pos - start_pos);
        }

        [[nodiscard]] std::size_t seq_length_with_start_end_adapters_trimmed()
            const {
            return get_seq_with_start_end_adapters_trimmed().size();
        }

        [[nodiscard]] std::string get_quals_with_start_end_adapters_trimmed()
            const {
            const auto [start_pos, end_pos] = trimmed_bounds(quals.size());
            return quals.substr(start_pos, end_pos - start_pos);
        }

        [[nodiscard]] std::vector<ReadPart> get_split_read_parts(
            int min_split_read_size) const {
            const std::string trimmed_seq =
                get_seq_with_start_end_adapters_trimmed();
            const std::string trimmed_quals =
                get_quals_with_start_end_adapters_trimmed();
            const auto min_size =
                static_cast<std::size_t>(std::max(0, min_split_read_size));

            std::vector<ReadPart> split_read_parts;
            std::string part_seq;
            std::string part_quals;

            for (std::size_t i = 0; i < trimmed_seq.size(); ++i) {
                if (middle_trim_positions.contains(static_cast<int>(i))) {
                    if (!part_seq.empty()) {
                        split_read_parts.emplace_back(part_seq, part_quals);
                        part_seq.clear();
                        part_quals.clear();
                    }
                    // Bases at trim positions are discarded, not appended
                    // to the next part (matches original Porechop behaviour).
                } else {
                    part_seq.push_back(trimmed_seq[i]);
                    if (i < trimmed_quals.size()) {
                        part_quals.push_back(trimmed_quals[i]);
                    }
                }
            }

            if (!part_seq.empty()) {
                split_read_parts.emplace_back(part_seq, part_quals);
            }

            split_read_parts.erase(
                std::remove_if(split_read_parts.begin(), split_read_parts.end(),
                               [min_size](const ReadPart& part) {
                                   return part.first.size() < min_size;
                               }),
                split_read_parts.end());
            return split_read_parts;
        }

        [[nodiscard]] std::string get_fasta(int min_split_read_size,
                                            bool discard_middle,
                                            bool untrimmed = false) const {
            if (middle_trim_positions.empty()) {
                std::string output_seq =
                    untrimmed ? seq : get_seq_with_start_end_adapters_trimmed();
                if (output_seq.empty()) {
                    return "";
                }
                restore_rna_for_output(output_seq);
                return ">" + name + "\n" + wrap_sequence(output_seq, 70);
            }

            if (discard_middle) {
                return "";
            }

            std::string fasta_str;
            const auto parts = get_split_read_parts(min_split_read_size);
            for (std::size_t i = 0; i < parts.size(); ++i) {
                if (parts[i].first.empty()) {
                    return "";
                }
                std::string output_seq = parts[i].first;
                restore_rna_for_output(output_seq);
                fasta_str +=
                    ">" +
                    add_number_to_read_name(name, static_cast<int>(i + 1)) +
                    "\n" + wrap_sequence(output_seq, 70);
            }
            return fasta_str;
        }

        [[nodiscard]] std::string get_fastq(int min_split_read_size,
                                            bool discard_middle,
                                            bool untrimmed = false) const {
            if (middle_trim_positions.empty()) {
                std::string output_seq =
                    untrimmed ? seq : get_seq_with_start_end_adapters_trimmed();
                std::string output_quals =
                    untrimmed ? quals
                              : get_quals_with_start_end_adapters_trimmed();
                if (output_seq.empty()) {
                    return "";
                }
                restore_rna_for_output(output_seq);
                return "@" + name + "\n" + output_seq + "\n+\n" + output_quals +
                       "\n";
            }

            if (discard_middle) {
                return "";
            }

            std::string fastq_str;
            const auto parts = get_split_read_parts(min_split_read_size);
            for (std::size_t i = 0; i < parts.size(); ++i) {
                std::string output_seq = parts[i].first;
                if (output_seq.empty()) {
                    return "";
                }
                restore_rna_for_output(output_seq);
                fastq_str +=
                    "@" +
                    add_number_to_read_name(name, static_cast<int>(i + 1)) +
                    "\n" + output_seq + "\n+\n" + parts[i].second + "\n";
            }
            return fastq_str;
        }

        // --- End trimming ---
        void find_start_trim(const std::vector<AdapterSet>& adapters,
                             int end_size, int extra_trim_size,
                             double end_threshold,
                             const ScoringScheme& scoring_scheme,
                             int min_trim_size, bool check_barcodes,
                             const std::string& barcode_dir = "");

        void find_end_trim(const std::vector<AdapterSet>& adapters,
                           int end_size, int extra_trim_size,
                           double end_threshold,
                           const ScoringScheme& scoring_scheme,
                           int min_trim_size, bool check_barcodes,
                           const std::string& barcode_dir = "");

        // --- Middle adapter detection ---
        void find_middle_adapters(
            const std::vector<AdapterSequence>& adapters,
            double middle_threshold, int extra_middle_trim_good_side,
            int extra_middle_trim_bad_side, const ScoringScheme& scoring_scheme,
            const std::set<std::string>& start_sequence_names,
            const std::set<std::string>& end_sequence_names);

        [[nodiscard]] static std::string add_number_to_read_name(
            const std::string& read_name, int number) {
            const std::string suffix = "_" + std::to_string(number);
            const std::size_t first_space = read_name.find(' ');
            if (first_space == std::string::npos) {
                return read_name + suffix;
            }
            std::string numbered = read_name;
            numbered.replace(first_space, 1, suffix + " ");
            return numbered;
        }

       private:
        [[nodiscard]] std::pair<std::size_t, std::size_t> trimmed_bounds(
            std::size_t length) const {
            const auto start = std::min<std::size_t>(
                static_cast<std::size_t>(std::max(0, start_trim_amount)),
                length);
            const auto end_trim = std::min<std::size_t>(
                static_cast<std::size_t>(std::max(0, end_trim_amount)),
                length - start);
            return {start, length - end_trim};
        }

        void restore_rna_for_output(std::string& output_seq) const {
            if (rna) {
                std::replace(output_seq.begin(), output_seq.end(), 'T', 'U');
            }
        }

        [[nodiscard]] static std::string wrap_sequence(
            const std::string& sequence, std::size_t width) {
            std::string wrapped;
            for (std::size_t pos = 0; pos < sequence.size(); pos += width) {
                wrapped += sequence.substr(
                    pos, std::min(width, sequence.size() - pos));
                wrapped += '\n';
            }
            return wrapped;
        }
    };

    inline std::string add_number_to_read_name(const std::string& read_name,
                                               int number) {
        return Read::add_number_to_read_name(read_name, number);
    }

}  // namespace porechop
