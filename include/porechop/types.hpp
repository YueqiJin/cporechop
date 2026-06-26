#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <string>

namespace porechop {

// =================================================================================================
// ScoringScheme: match/mismatch/gap scoring parameters
// =================================================================================================
struct ScoringScheme {
    int match = 3;
    int mismatch = -6;
    int gap_open = -5;
    int gap_extend = -2;

    /// Parse a comma-separated "match,mismatch,gap_open,gap_extend" string.
    /// Returns std::nullopt on malformed input.
    static std::optional<ScoringScheme> parse(const std::string& scoring_str) {
        std::istringstream iss(scoring_str);
        int m, mm, go, ge;
        char comma;
        if (iss >> m >> comma >> mm >> comma >> go >> comma >> ge &&
            iss.eof()) {
            return ScoringScheme{m, mm, go, ge};
        }
        return std::nullopt;
    }
};

// =================================================================================================
// AdapterSequence: a named adapter sequence (used inside AdapterSet)
// =================================================================================================
struct AdapterSequence {
    std::string name;
    std::string sequence;
};

// =================================================================================================
// AdapterSet: corresponds to original Porechop's Adapter class
// =================================================================================================
struct AdapterSet {
    std::string name;
    AdapterSequence start_sequence;
    AdapterSequence end_sequence;
    double best_start_score = 0.0;
    double best_end_score = 0.0;

    // --------------------------------------------------------------------------------------------
    // Queries
    // --------------------------------------------------------------------------------------------

    /// Returns true if this adapter set is a barcode (name starts with "Barcode
    /// ").
    bool is_barcode() const { return name.find("Barcode ") == 0; }

    /// Returns "reverse" if name contains "(reverse)", otherwise "forward".
    std::string barcode_direction() const {
        if (name.find("(reverse)") != std::string::npos) return "reverse";
        return "forward";
    }

    /// Extracts the barcode short name from a name like "Barcode 01 (forward)"
    /// -> "BC01".
    std::string get_barcode_name() const {
        static const std::string prefix = "Barcode ";
        if (name.find(prefix) != 0) return "BC??";

        std::size_t num_start = prefix.size();
        std::size_t num_end = name.find(' ', num_start);
        if (num_end == std::string::npos) num_end = name.size();

        std::string num_str = name.substr(num_start, num_end - num_start);
        int num = std::atoi(num_str.c_str());

        std::string result = "BC";
        if (num < 10) result += '0';
        result += std::to_string(num);
        return result;
    }

    /// Returns the higher of best_start_score and best_end_score.
    double best_start_or_end_score() const {
        return std::max(best_start_score, best_end_score);
    }

    // --------------------------------------------------------------------------------------------
    // Factory
    // --------------------------------------------------------------------------------------------

    /// Create an AdapterSet with the given name and optional start/end
    /// sequences. Empty @p start_name or @p end_name means that side is absent.
    static AdapterSet create(const std::string& name,
                             const std::string& start_name,
                             const std::string& start_seq,
                             const std::string& end_name,
                             const std::string& end_seq) {
        AdapterSet set;
        set.name = name;
        if (!start_name.empty()) {
            set.start_sequence = AdapterSequence{start_name, start_seq};
        }
        if (!end_name.empty()) {
            set.end_sequence = AdapterSequence{end_name, end_seq};
        }
        set.best_start_score = 0.0;
        set.best_end_score = 0.0;
        return set;
    }
};

// =================================================================================================
// PercentIdentity alias
// =================================================================================================
using PercentIdentity = double;

// =================================================================================================
// AdapterAlignment: result of aligning an adapter against a read end
// =================================================================================================
struct AdapterAlignment {
    int read_begin_pos = -1;  // -1 means no alignment found
    int read_end_pos = -1;
    int adapter_begin_pos = -1;
    int adapter_end_pos = -1;
    int raw_score = 0;
    PercentIdentity aligned_region_percent_id = 0.0;
    PercentIdentity full_adapter_percent_id = 0.0;
};

}  // namespace porechop
