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

#include "porechop/adapters.hpp"
#include "porechop/types.hpp"

namespace porechop {

    // =================================================================================================
    // load_adapter_file: parse a pipe-delimited adapter file.
    //
    // Format per line (pipe-delimited, 3 fields):
    //   name | start_name,start_seq | end_name,end_seq
    //
    // Lines starting with '#' are comments.  Blank lines are skipped.
    // Malformed lines print to std::cerr and continue.
    // Throws std::runtime_error if the file cannot be opened.
    // =================================================================================================
    std::vector<AdapterSet> load_adapter_file(const std::string& path);

    // =================================================================================================
    // AdapterRegistry: manages a collection of adapter sets, supporting
    // built-in loading, individual/addition, file-merging, and search
    // filtering.
    // =================================================================================================
    class AdapterRegistry {
       public:
        /// Construct an empty registry.
        AdapterRegistry() = default;

        /// Construct a registry pre-populated with the given adapters.
        explicit AdapterRegistry(std::vector<AdapterSet> adapters)
            : adapters_(std::move(adapters)) {}

        /// Return a registry containing all 119 built-in adapter sets.
        static AdapterRegistry get_builtin();

        /// Add a single adapter set.
        void add(const AdapterSet& adapter);

        /// Load adapter sets from a file and merge them into the registry.
        /// Existing entries (by name) take priority over file entries.
        void add_file(const std::string& path);

        /// Return ALL adapter sets in the registry (including full-sequence
        /// ones).
        std::vector<AdapterSet> get_all() const;

        /// Return only adapter sets whose names do NOT contain "(full
        /// sequence)".
        std::vector<AdapterSet> get_search() const;

        /// Return true if the registry holds no adapters.
        bool empty() const;

        /// Return the number of adapter sets.
        std::size_t size() const;

       private:
        std::vector<AdapterSet> adapters_;
    };

}  // namespace porechop
