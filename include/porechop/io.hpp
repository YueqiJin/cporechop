#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <iosfwd>
#include <string>
#include <vector>

#include "porechop/read.hpp"

namespace porechop {

    /// Load a FASTA file and return a vector of Reads.
    /// Lines starting with '>' start new records.
    /// Sequence lines are concatenated.
    /// Empty lines are skipped.
    std::vector<Read> load_fasta(const std::string& path);

    /// Load a FASTQ file and return a vector of Reads.
    /// Expects 4-line blocks: @name, sequence, +, qualities.
    /// Windows CR characters (\r) are stripped.
    std::vector<Read> load_fastq(const std::string& path);

    /// Detect whether a file is FASTA or FASTQ by examining the first
    /// character. Returns "FASTA" if first char is '>', "FASTQ" if first char
    /// is '@'. Throws std::runtime_error if the file cannot be identified.
    std::string detect_read_type(const std::string& path);

    /// Load reads from a file, auto-detecting the format (FASTA or FASTQ).
    /// Delegates to load_fasta or load_fastq based on detect_read_type.
    std::vector<Read> load_reads(const std::string& path);

    /// Write Reads in FASTA format to an output stream.
    /// Each read is formatted via Read::get_fasta(untrimmed=true).
    void write_fasta(const std::vector<Read>& reads, std::ostream& os);

    /// Write Reads in FASTQ format to an output stream.
    /// Each read is formatted via Read::get_fastq(untrimmed=true).
    void write_fastq(const std::vector<Read>& reads, std::ostream& os);

}  // namespace porechop
