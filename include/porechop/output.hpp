#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <string>
#include <vector>

#include "porechop/read.hpp"

namespace porechop {

    /// Write trimmed reads to file, stdout, or barcode-specific bin files.
    ///
    /// @param reads               All reads to output.
    /// @param format              "auto", "fasta", "fastq", "fasta.gz", or
    /// "fastq.gz".
    ///                            "auto" derives the format from the @p
    ///                            output_file extension (or defaults to "fastq"
    ///                            for stdout).
    /// @param output_file         Destination file path.  Empty string means
    /// stdout.
    /// @param barcode_dir         Destination directory for barcode bin files.
    ///                            Empty string means no barcode binning (single
    ///                            output).
    /// @param min_split_read_size Minimum fragment size for split reads.
    /// @param discard_middle      If true, skip reads that have middle adapter
    /// hits.
    /// @param untrimmed           If true, output the original (untrimmed)
    /// sequence.
    /// @param discard_unassigned  If true (and barcode_dir is set), skip reads
    /// whose
    ///                            barcode_call is "none".
    void output_reads(const std::vector<Read>& reads, const std::string& format,
                      const std::string& output_file,
                      const std::string& barcode_dir, int min_split_read_size,
                      bool discard_middle, bool untrimmed,
                      bool discard_unassigned, int verbosity = 1);

    /// Display the adapter set results table showing best start/end %ID for
    /// each adapter set.  Matching sets (those above the detection threshold)
    /// are highlighted in green.  Prints to stderr when verbosity >= 1.
    void display_adapter_set_results(
        const std::vector<AdapterSet>& scored_adapters,
        const std::vector<AdapterSet>& matching_sets, int verbosity);

    /// Display the "Trimming adapters from read ends" header and list the
    /// adapter sequences that will be searched for (in red, matching original
    /// Porechop).  Prints to stderr when verbosity >= 1.
    void display_trimming_header_and_adapters(
        const std::vector<AdapterSet>& matching_sets, int verbosity);

}  // namespace porechop
