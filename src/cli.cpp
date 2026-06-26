/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/cli.hpp"

#include <CLI/CLI.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace porechop {

    // =================================================================================================
    // PipelineOptions constructor: set threads to min(hardware_concurrency(),
    // 16)
    // =================================================================================================
    PipelineOptions::PipelineOptions() {
        unsigned int hwc = std::thread::hardware_concurrency();
        threads = static_cast<int>(std::min(hwc == 0 ? 1u : hwc, 16u));
    }

    // =================================================================================================
    // parse_args
    // =================================================================================================
    PipelineOptions parse_args(int argc, const char* const* argv) {
        // ------------------------------------------------------------------
        // 1. Build a temporary options object; user-supplied CLI flags
        //    overwrite these defaults via reference binding.
        // ------------------------------------------------------------------
        PipelineOptions opts;

        CLI::App app{
            "Porechop: a tool for finding adapters in Oxford Nanopore reads, "
            "trimming them from the ends and splitting reads with internal "
            "adapters"};
        auto fmt = std::make_shared<CLI::Formatter>();
        fmt->column_width(35);
        app.formatter(fmt);
        app.option_defaults()->always_capture_default(true);

        // ---- Main options ----
        app.add_option("-i,--input", opts.input, "FASTA/FASTQ of input reads")
            ->required();

        app.add_option("-o,--output", opts.output,
                       "Filename for FASTA or FASTQ of trimmed reads (if not "
                       "set, trimmed reads "
                       "will be printed to stdout)");

        std::vector<std::string> format_choices = {"auto", "fasta", "fastq",
                                                   "fasta.gz", "fastq.gz"};
        app.add_option("--format", opts.format,
                       "Output format for the reads - if auto, the format will "
                       "be chosen based on "
                       "the output filename or the input read format")
            ->check(CLI::IsMember(format_choices));

        app.add_option("-v,--verbosity", opts.verbosity,
                       "Level of progress information: 0 = none, 1 = some, 2 = "
                       "lots, 3 = full")
            ->check(CLI::Range(0, 3));

        app.add_option("-t,--threads", opts.threads,
                       "Number of threads to use for adapter alignment");

        // ---- Barcode binning settings ----
        app.add_option("-b,--barcode_dir", opts.barcode_dir,
                       "Reads will be binned based on their barcode and saved "
                       "to separate files "
                       "in this directory (incompatible with --output)");

        app.add_option("--barcode_threshold", opts.barcode_threshold,
                       "A read must have at least this percent identity to a "
                       "barcode to be binned");

        app.add_option(
            "--barcode_diff", opts.barcode_diff,
            "If the difference between a read's best barcode identity and its "
            "second-best barcode identity is less than this value, it will not "
            "be "
            "put in a barcode bin");

        app.add_flag("--require_two_barcodes", opts.require_two_barcodes,
                     "Reads will only be put in barcode bins if they have a "
                     "strong match for "
                     "the barcode on both their start and end");

        app.add_flag("--untrimmed", opts.untrimmed,
                     "Bin reads but do not trim them");

        app.add_flag(
            "--discard_unassigned", opts.discard_unassigned,
            "Discard unassigned reads (instead of creating a \"none\" bin)");

        // ---- Adapter search settings ----
        app.add_option("--adapter_threshold", opts.adapter_threshold,
                       "An adapter set has to have at least this percent "
                       "identity to be labelled "
                       "as present and trimmed off (0 to 100)");

        app.add_option("--check_reads", opts.check_reads,
                       "This many reads will be aligned to all possible "
                       "adapters to determine "
                       "which adapter sets are present");

        app.add_option("--scoring_scheme", opts.scoring_scheme_str,
                       "Comma-delimited string of alignment scores: "
                       "match,mismatch,gap_open,gap_extend");

        // ---- NEW: runtime adapter file ----
        app.add_option("--adapter_file", opts.adapter_file,
                       "File containing custom adapter sequences to use in "
                       "addition to built-in "
                       "adapters (pipe-delimited format: "
                       "name|start_name,seq|end_name,seq)");

        app.add_flag("--no_default_adapters", opts.no_default_adapters,
                     "Use only adapters from --adapter_file, skipping all "
                     "built-in adapter sets "
                     "(requires --adapter_file)");

        // ---- End adapter settings ----
        app.add_option("--end_size", opts.end_size,
                       "The number of base pairs at each end of the read which "
                       "will be searched "
                       "for adapter sequences");

        app.add_option("--min_trim_size", opts.min_trim_size,
                       "Adapter alignments smaller than this will be ignored");

        app.add_option("--extra_end_trim", opts.extra_end_trim,
                       "This many additional bases will be removed next to "
                       "adapters found at "
                       "the ends of reads");

        app.add_option("--end_threshold", opts.end_threshold,
                       "Adapters at the ends of reads must have at least this "
                       "percent identity "
                       "to be removed (0 to 100)");

        // ---- Middle adapter settings ----
        app.add_flag("--no_split", opts.no_split,
                     "Skip splitting reads based on middle adapters");

        app.add_flag("--discard_middle", opts.discard_middle,
                     "Reads with middle adapters will be discarded");

        app.add_option("--middle_threshold", opts.middle_threshold,
                       "Adapters in the middle of reads must have at least "
                       "this percent identity "
                       "to be found (0 to 100)");

        app.add_option("--extra_middle_trim_good_side",
                       opts.extra_middle_trim_good_side,
                       "This many additional bases will be removed next to "
                       "middle adapters on "
                       "their \"good\" side");

        app.add_option("--extra_middle_trim_bad_side",
                       opts.extra_middle_trim_bad_side,
                       "This many additional bases will be removed next to "
                       "middle adapters on "
                       "their \"bad\" side");

        app.add_option("--min_split_read_size", opts.min_split_read_size,
                       "Post-split read pieces smaller than this many base "
                       "pairs will not be "
                       "outputted");

        // ---- Version ----
        app.set_version_flag("--version", "1.0.0",
                             "Show program's version number and exit");

        // ------------------------------------------------------------------
        // 2. Parse
        // ------------------------------------------------------------------
        try {
            app.parse(argc, argv);
        } catch (const CLI::CallForHelp&) {
            throw cli_info(app.help());
        } catch (const CLI::CallForVersion&) {
            throw cli_info(app.version());
        } catch (const CLI::ParseError& e) {
            throw std::runtime_error(e.what());
        }

        // ------------------------------------------------------------------
        // 3. Post-parse validation
        // ------------------------------------------------------------------

        // Threads must be >= 1
        if (opts.threads < 1) {
            throw std::runtime_error("Error: at least one thread required");
        }

        // -o and -b are mutually exclusive
        if (!opts.barcode_dir.empty() && !opts.output.empty()) {
            throw std::runtime_error(
                "Error: only one of the following options may be used: "
                "--output, --barcode_dir");
        }

        // --no_default_adapters requires --adapter_file
        if (opts.no_default_adapters && opts.adapter_file.empty()) {
            throw std::runtime_error(
                "Error: --no_default_adapters requires --adapter_file");
        }

        // --untrimmed requires -b
        if (opts.untrimmed && opts.barcode_dir.empty()) {
            throw std::runtime_error(
                "Error: --untrimmed can only be used with --barcode_dir");
        }

        // Barcode output auto-enables discard_middle
        if (!opts.barcode_dir.empty()) {
            opts.discard_middle = true;
        }

        // Parse the scoring scheme
        auto parsed = ScoringScheme::parse(opts.scoring_scheme_str);
        if (!parsed.has_value()) {
            throw std::runtime_error(
                "Error: incorrectly formatted scoring scheme");
        }
        opts.scoring_scheme_vals = *parsed;

        // Determine print_to_stderr: when no -o and no -b, output goes to
        // stdout, so progress messages go to stderr instead.
        opts.print_to_stderr =
            (opts.output.empty() && opts.barcode_dir.empty());

        return opts;
    }

}  // namespace porechop
