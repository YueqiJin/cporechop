/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <CLI/CLI.hpp>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>

#include "porechop/cli.hpp"
#include "porechop/pipeline.hpp"

// =================================================================================================
// ANSI escape codes (matching original Porechop misc.py:270-308)
// =================================================================================================
namespace {
    constexpr const char* END = "\033[0m";
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* UNDL = "\033[4m";
    constexpr const char* GRN = "\033[32m";

    /// Apply ANSI codes to a string (returns string, used inline).
    std::string bold_underline(const std::string& s) {
        return std::string(BOLD) + UNDL + s + END;
    }
    std::string green(const std::string& s) { return GRN + s + END; }

    /// Add thousands separators (mimics original int_to_str).
    std::string int_to_str(int val) {
        std::string s = std::to_string(val);
        int insert_pos = static_cast<int>(s.size()) - 3;
        while (insert_pos > 0) {
            s.insert(static_cast<std::size_t>(insert_pos), ",");
            insert_pos -= 3;
        }
        return s;
    }
}  // anonymous namespace

// =================================================================================================
// Verbose output helpers (verbosity 1+)
// =================================================================================================
namespace {

    void print_loading_reads(std::ostream& print_dest,
                             const std::string& input_path) {
        print_dest << bold_underline("Loading reads") << '\n';
        print_dest << "  " << input_path << '\n';
    }

    void print_read_count(std::ostream& print_dest, std::size_t count) {
        print_dest << int_to_str(static_cast<int>(count))
                   << " reads loaded\n\n";
    }

    void print_summary(std::ostream& print_dest,
                       const porechop::PipelineResult& res, int verbosity) {
        if (verbosity < 1) return;

        if (res.trimmed_start_count > 0) {
            print_dest << "  "
                       << green(int_to_str(
                              static_cast<int>(res.trimmed_start_count)))
                       << " / " << int_to_str(static_cast<int>(res.read_count))
                       << " reads had adapters trimmed from their start ("
                       << int_to_str(static_cast<int>(res.start_bp_removed))
                       << " bp removed)\n";
        } else {
            print_dest << "  "
                       << int_to_str(static_cast<int>(res.trimmed_start_count))
                       << " / " << int_to_str(static_cast<int>(res.read_count))
                       << " reads had adapters trimmed from their start\n";
        }

        if (res.trimmed_end_count > 0) {
            print_dest << "  "
                       << green(int_to_str(
                              static_cast<int>(res.trimmed_end_count)))
                       << " / " << int_to_str(static_cast<int>(res.read_count))
                       << " reads had adapters trimmed from their end ("
                       << int_to_str(static_cast<int>(res.end_bp_removed))
                       << " bp removed)\n";
        } else {
            print_dest << "  "
                       << int_to_str(static_cast<int>(res.trimmed_end_count))
                       << " / " << int_to_str(static_cast<int>(res.read_count))
                       << " reads had adapters trimmed from their end\n";
        }

        // Middle adapter summary
        print_dest << '\n'
                   << bold_underline(
                          "Splitting reads containing middle adapters")
                   << '\n';
        if (res.split_count > 0) {
            print_dest << "  " << int_to_str(static_cast<int>(res.split_count))
                       << " / " << int_to_str(static_cast<int>(res.read_count))
                       << " reads were split based on middle adapters\n";
        } else {
            print_dest << "  " << int_to_str(static_cast<int>(res.split_count))
                       << " / " << int_to_str(static_cast<int>(res.read_count))
                       << " reads were split based on middle adapters\n";
        }

        // Barcode summary
        if (!res.barcode_counts.empty()) {
            print_dest << '\n' << bold_underline("Barcode binning") << '\n';
            for (const auto& [barcode, count] : res.barcode_counts) {
                print_dest << "  " << barcode << ": "
                           << int_to_str(static_cast<int>(count)) << " reads\n";
            }
        }

        print_dest << '\n';
    }

}  // anonymous namespace

// =================================================================================================
// main
// =================================================================================================
int main(int argc, const char* const* argv) {
    try {
        // Parse CLI arguments
        porechop::PipelineOptions opts = porechop::parse_args(argc, argv);

        // Determine print destination: stderr when output goes to stdout,
        // otherwise stdout (for progress messages).
        std::ostream& print_dest = opts.print_to_stderr ? std::cerr : std::cout;

        // Verbosity 1+: loading progress
        if (opts.verbosity >= 1) {
            print_loading_reads(print_dest, opts.input);
        }

        // Run pipeline
        porechop::PipelineResult result = porechop::run_pipeline(opts);

        // Verbosity 1+: print read count and summary
        if (opts.verbosity >= 1) {
            print_read_count(print_dest, result.read_count);
            print_summary(print_dest, result, opts.verbosity);
        }

        return 0;
    } catch (const porechop::cli_info& e) {
        // --help or --version: print message and exit 0
        std::cout << e.what() << '\n';
        return 0;
    } catch (const std::exception& e) {
        // All other errors: print to stderr and exit 1
        std::cerr << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Error: unknown exception\n";
        return 1;
    }
}
