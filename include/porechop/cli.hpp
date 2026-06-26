#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <stdexcept>
#include <string>
#include <vector>

#include "porechop/types.hpp"

namespace porechop {

    // cli_info — thrown on --help or --version (caller should exit 0 with
    // printed message).
    class cli_info : public std::exception {
        std::string msg_;

       public:
        explicit cli_info(std::string msg) : msg_(std::move(msg)) {}
        const char* what() const noexcept override { return msg_.c_str(); }
    };

    // =================================================================================================
    // PipelineOptions: all CLI settings aggregated into one struct.
    // =================================================================================================
    struct PipelineOptions {
        // Main options
        std::string input;
        std::string output;
        std::string format = "auto";
        int verbosity = 1;
        int threads =
            1;  // set to std::min(hardware_concurrency(), 16) in constructor

        // Barcode binning settings
        std::string barcode_dir;
        double barcode_threshold = 75.0;
        double barcode_diff = 5.0;
        bool require_two_barcodes = false;
        bool untrimmed = false;
        bool discard_unassigned = false;

        // Adapter search settings
        double adapter_threshold = 90.0;
        int check_reads = 10000;
        std::string scoring_scheme_str = "3,-6,-5,-2";
        std::string adapter_file;          // NEW — runtime adapter file
        bool no_default_adapters = false;  // NEW — skip built-in adapters

        // End adapter settings
        int end_size = 150;
        int min_trim_size = 4;
        int extra_end_trim = 2;
        double end_threshold = 75.0;

        // Middle adapter settings
        bool no_split = false;
        bool discard_middle = false;
        double middle_threshold = 90.0;
        int extra_middle_trim_good_side = 10;
        int extra_middle_trim_bad_side = 100;
        int min_split_read_size = 1000;

        // Computed fields (set during/after parsing)
        bool print_to_stderr = true;
        ScoringScheme scoring_scheme_vals;

        PipelineOptions();
    };

    // =================================================================================================
    // parse_args: entry point for CLI11 argument parsing.
    // Throws cli_info (exit 0) on --help/--version,
    //         CLI::ParseError on malformed arguments,
    //         std::runtime_error on validation errors.
    // =================================================================================================
    PipelineOptions parse_args(int argc, const char* const* argv);

}  // namespace porechop
