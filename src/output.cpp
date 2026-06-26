/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/output.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace porechop {

    // ANSI formatting codes (matching original Porechop misc.py:270-308).
    namespace {
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* UNDERLINE = "\033[4m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* END = "\033[0m";
    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------------------------

    namespace {

        /// Case-insensitive suffix check.
        bool ends_with_ci(const std::string& str, const std::string& suffix) {
            if (suffix.size() > str.size()) return false;
            return std::equal(
                suffix.rbegin(), suffix.rend(), str.rbegin(),
                [](char a, char b) { return ::tolower(a) == ::tolower(b); });
        }

        /// Add thousands separators to an integer (mimics original Porechop
        /// int_to_str).
        std::string int_to_str(int val) {
            std::string s = std::to_string(val);
            int insert_pos = static_cast<int>(s.size()) - 3;
            while (insert_pos > 0) {
                s.insert(static_cast<std::size_t>(insert_pos), ",");
                insert_pos -= 3;
            }
            return s;
        }

        /// Resolve the output format: strip trailing ".gz" to detect gzip,
        /// convert "auto" to a concrete format derived from the output file
        /// extension. Returns {concrete_format, is_gzipped}.
        std::pair<std::string, bool> resolve_format(
            const std::string& format, const std::string& output_file,
            const std::string& barcode_dir) {
            std::string fmt = format;
            bool gzipped = false;

            // Detect explicit .gz suffix
            if (ends_with_ci(fmt, ".gz")) {
                gzipped = true;
                fmt = fmt.substr(0, fmt.size() - 3);
            }

            if (fmt == "auto") {
                if (!output_file.empty()) {
                    std::string lower = output_file;
                    std::transform(
                        lower.begin(), lower.end(), lower.begin(),
                        [](char c) { return static_cast<char>(::tolower(c)); });

                    if (lower.find(".fasta.gz") != std::string::npos) {
                        fmt = "fasta";
                        gzipped = true;
                    } else if (lower.find(".fastq.gz") != std::string::npos) {
                        fmt = "fastq";
                        gzipped = true;
                    } else if (lower.find(".fasta") != std::string::npos) {
                        fmt = "fasta";
                    } else if (lower.find(".fastq") != std::string::npos) {
                        fmt = "fastq";
                    } else {
                        fmt = "fastq";  // fallback
                    }
                } else {
                    // stdout — default to fastq
                    fmt = "fastq";
                }
            }

            // When producing barcode bin files, also check if output_file
            // implies gzip (original: "if barcode_dir is not None and
            // input_filename.lower().endswith('.gz')")
            if (barcode_dir.empty() && !output_file.empty()) {
                // gzip flag is already set above for the single-file path; keep
                // it
            }

            return {fmt, gzipped};
        }

        /// Run the gzip command to compress a file (replacing it in-place with
        /// .gz). Returns true on success.
        bool gzip_file(const std::string& path) {
            std::string cmd = "gzip -f \"" + path + "\"";
            int ret = std::system(cmd.c_str());
            return ret == 0;
        }

        /// Run "gzip -c <input> > <output>" to compress and redirect.
        bool gzip_to_output(const std::string& input_path,
                            const std::string& output_path) {
            std::string cmd =
                "gzip -c \"" + input_path + "\" > \"" + output_path + "\"";
            int ret = std::system(cmd.c_str());
            return ret == 0;
        }

        /// Format a single read string based on output format.
        std::string format_read(const Read& read, const std::string& fmt,
                                int min_split_read_size, bool discard_middle,
                                bool untrimmed) {
            if (fmt == "fasta") {
                return read.get_fasta(min_split_read_size, discard_middle,
                                      untrimmed);
            }
            return read.get_fastq(min_split_read_size, discard_middle,
                                  untrimmed);
        }

        /// Print a formatted table to stderr (matching original print_table).
        void print_table(const std::vector<std::vector<std::string>>& rows,
                         const std::string& alignments = "LRRL",
                         int max_col_width = 60, int col_separation = 2,
                         const std::set<std::size_t>& green_rows = {}) {
            if (rows.empty()) return;

            // Calculate column widths
            std::vector<std::size_t> col_widths(rows[0].size(), 0);
            for (const auto& row : rows) {
                for (std::size_t i = 0; i < row.size() && i < col_widths.size();
                     ++i) {
                    col_widths[i] = std::max(col_widths[i], row[i].size());
                }
            }

            // Clamp to max_col_width
            for (auto& w : col_widths) {
                w = std::min(w, static_cast<std::size_t>(max_col_width));
            }

            auto pad = [&](const std::string& text, std::size_t col,
                           bool right_align) -> std::string {
                std::string cell = text;
                if (cell.size() > col_widths[col]) {
                    cell = cell.substr(0, col_widths[col]);
                }
                if (right_align) {
                    return std::string(col_widths[col] - cell.size(), ' ') +
                           cell;
                }
                return cell + std::string(col_widths[col] - cell.size(), ' ');
            };

            const std::string sep(col_separation, ' ');

            for (std::size_t row_idx = 0; row_idx < rows.size(); ++row_idx) {
                const auto& row = rows[row_idx];
                bool is_green = green_rows.count(row_idx) > 0;
                if (is_green) std::cerr << GREEN;
                for (std::size_t i = 0; i < row.size(); ++i) {
                    if (i > 0) std::cerr << sep;
                    bool right_align =
                        (i < alignments.size() && alignments[i] == 'R');
                    std::cerr << pad(row[i], i, right_align);
                }
                if (is_green) std::cerr << END;
                std::cerr << '\n';
            }
        }

    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // display_adapter_set_results
    // ---------------------------------------------------------------------------

    void display_adapter_set_results(
        const std::vector<AdapterSet>& scored_adapters,
        const std::vector<AdapterSet>& matching_sets, int verbosity) {
        if (verbosity < 1) return;

        // Build the set of matching adapter names for green highlighting.
        std::set<std::string> matching_names;
        for (const auto& as : matching_sets) {
            matching_names.insert(as.name);
        }

        std::vector<std::vector<std::string>> table = {
            {"Set", "Best read start %ID", "Best read end %ID"}};
        std::set<std::size_t> green_rows;

        for (const auto& adapter_set : scored_adapters) {
            // Skip full-sequence adapters (matching original Porechop).
            if (adapter_set.name.find("(full sequence)") != std::string::npos) {
                continue;
            }

            std::ostringstream start_ss, end_ss;
            start_ss << std::fixed << std::setprecision(1)
                     << adapter_set.best_start_score;
            end_ss << std::fixed << std::setprecision(1)
                   << adapter_set.best_end_score;

            table.push_back(
                {adapter_set.name, start_ss.str(), end_ss.str()});

            if (matching_names.count(adapter_set.name) > 0) {
                green_rows.insert(table.size() - 1);
            }
        }

        print_table(table, "LRR", 60, 2, green_rows);
    }

    // ---------------------------------------------------------------------------
    // display_trimming_header_and_adapters
    // ---------------------------------------------------------------------------

    void display_trimming_header_and_adapters(
        const std::vector<AdapterSet>& matching_sets, int verbosity) {
        if (verbosity < 1) return;

        std::cerr << BOLD << UNDERLINE << "Trimming adapters from read ends"
                  << END << '\n';

        // Find the longest adapter name for right-alignment.
        std::size_t name_len = 0;
        for (const auto& as : matching_sets) {
            if (!as.start_sequence.name.empty()) {
                name_len = std::max(name_len, as.start_sequence.name.size());
            }
            if (!as.end_sequence.name.empty()) {
                name_len = std::max(name_len, as.end_sequence.name.size());
            }
        }

        for (const auto& as : matching_sets) {
            if (!as.start_sequence.sequence.empty()) {
                std::cerr << "  "
                          << std::string(name_len - as.start_sequence.name.size(),
                                         ' ')
                          << as.start_sequence.name << ": " << RED
                          << as.start_sequence.sequence << END << '\n';
            }
            if (!as.end_sequence.sequence.empty()) {
                std::cerr << "  "
                          << std::string(name_len - as.end_sequence.name.size(),
                                         ' ')
                          << as.end_sequence.name << ": " << RED
                          << as.end_sequence.sequence << END << '\n';
            }
        }
        std::cerr << '\n';
    }

    // ---------------------------------------------------------------------------
    // output_reads
    // ---------------------------------------------------------------------------

    void output_reads(const std::vector<Read>& reads, const std::string& format,
                      const std::string& output_file,
                      const std::string& barcode_dir, int min_split_read_size,
                      bool discard_middle, bool untrimmed,
                      bool discard_unassigned, int verbosity) {
        // --- Format resolution
        // ------------------------------------------------
        auto [fmt, gzipped_out] =
            resolve_format(format, output_file, barcode_dir);

        const bool to_stdout = output_file.empty() && barcode_dir.empty();

        // --- Progress message (verbosity >= 1)
        // ---------------------------------
        if (verbosity >= 1) {
            const char* action = untrimmed ? "untrimmed" : "trimmed";
            std::string destination;
            if (!barcode_dir.empty()) {
                destination = "barcode-specific files";
            } else if (to_stdout) {
                destination = "stdout";
            } else {
                destination = "file";
            }
            std::cerr << BOLD << UNDERLINE << "Saving " << action
                      << " reads to " << destination << END << '\n';
        }

        // --- Barcode binning
        // --------------------------------------------------
        if (!barcode_dir.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(barcode_dir, ec);

            std::map<std::string, std::ofstream> barcode_files;
            std::map<std::string, int> barcode_read_counts;
            std::map<std::string, int> barcode_base_counts;

            for (const auto& read : reads) {
                const std::string& barcode_name = read.barcode_call;

                if (discard_unassigned && barcode_name == "none") {
                    continue;
                }

                std::string read_str = format_read(
                    read, fmt, min_split_read_size, discard_middle, untrimmed);
                if (read_str.empty()) {
                    continue;
                }

                // Lazily open barcode file
                if (barcode_files.find(barcode_name) == barcode_files.end()) {
                    std::string filepath = (std::filesystem::path(barcode_dir) /
                                            (barcode_name + "." + fmt))
                                               .string();
                    barcode_files[barcode_name].open(filepath);
                }

                barcode_files[barcode_name] << read_str;
                barcode_read_counts[barcode_name]++;

                // Base count: use original or trimmed depending on untrimmed
                // flag
                int seq_length =
                    untrimmed
                        ? static_cast<int>(read.seq.size())
                        : static_cast<int>(
                              read.seq_length_with_start_end_adapters_trimmed());
                barcode_base_counts[barcode_name] += seq_length;
            }

            // Close all files
            for (auto& [name, file] : barcode_files) {
                file.close();
            }

            // Gzip individual files if needed
            if (gzipped_out) {
                for (auto& [name, file] : barcode_files) {
                    (void)file;
                    std::string bin_path = (std::filesystem::path(barcode_dir) /
                                            (name + "." + fmt))
                                               .string();
                    if (std::filesystem::exists(bin_path)) {
                        gzip_file(bin_path);
                    }
                }
            }

            // Build and print summary table
            std::vector<std::vector<std::string>> table = {
                {"Barcode", "Reads", "Bases", "File"}};

            for (const auto& [barcode_name, count] : barcode_read_counts) {
                std::string bin_filename =
                    (std::filesystem::path(barcode_dir) /
                     (barcode_name + "." + fmt + (gzipped_out ? ".gz" : "")))
                        .string();

                table.push_back({barcode_name, int_to_str(count),
                                 int_to_str(barcode_base_counts[barcode_name]),
                                 bin_filename});
            }

            std::cerr << '\n';
            print_table(table, "LRRL", 60, 2);
            std::cerr << '\n';
            return;
        }

        // --- Single output (stdout or file) ----------------------------------
        if (to_stdout) {
            for (const auto& read : reads) {
                std::string read_str = format_read(
                    read, fmt, min_split_read_size, discard_middle, untrimmed);
                if (!read_str.empty()) {
                    std::cout << read_str;
                }
            }
            if (verbosity >= 1) {
                std::cerr << "Done\n";
            }
            return;
        }

        // File output
        std::string out_filename;
        std::string gzip_temp;

        if (gzipped_out) {
            static int gzip_counter = 0;
            ++gzip_counter;
            gzip_temp =
                (std::filesystem::temp_directory_path() /
                 ("cporechop_out_" + std::to_string(gzip_counter) + "." + fmt))
                    .string();
            out_filename = gzip_temp;
        } else {
            out_filename = output_file;
        }

        {
            std::ofstream out(out_filename);
            if (!out.is_open()) {
                throw std::runtime_error("Could not open output file: " +
                                         out_filename);
            }
            for (const auto& read : reads) {
                std::string read_str = format_read(
                    read, fmt, min_split_read_size, discard_middle, untrimmed);
                out << read_str;
            }
        }

        if (gzipped_out) {
            gzip_to_output(gzip_temp, output_file);
            std::error_code ec;
            std::filesystem::remove(gzip_temp, ec);
        }

        if (verbosity >= 1) {
            std::cerr << "\nSaved result to "
                      << std::filesystem::absolute(output_file).string()
                      << '\n';
        }
    }

}  // namespace porechop
