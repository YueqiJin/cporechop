/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/adapter_file.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace porechop {

    // =================================================================================================
    // Internal helpers
    // =================================================================================================
    namespace {

        /// Trim leading and trailing whitespace from a string.
        std::string trim(const std::string& s) {
            const auto start = s.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return {};
            const auto end = s.find_last_not_of(" \t\r\n");
            return s.substr(start, end - start + 1);
        }

    }  // anonymous namespace

    // =================================================================================================
    // load_adapter_file
    // =================================================================================================
    std::vector<AdapterSet> load_adapter_file(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open adapter file: " + path);
        }

        std::vector<AdapterSet> result;
        std::string line;
        int line_num = 0;

        while (std::getline(file, line)) {
            ++line_num;

            // Trim leading/trailing whitespace
            line = trim(line);

            // Skip blank lines and comments
            if (line.empty() || line[0] == '#') continue;

            // Validate: must have exactly 2 pipe characters (3 fields)
            int pipe_count = 0;
            for (char c : line) {
                if (c == '|') ++pipe_count;
            }
            if (pipe_count != 2) {
                std::cerr << "Error: malformed line " << line_num
                          << " in adapter file (expected 3 pipe-delimited "
                             "fields, found "
                          << (pipe_count + 1) << ")\n";
                continue;
            }

            // Split on '|' using stringstream
            std::istringstream ss(line);
            std::string name, start_field, end_field;

            // Field 1: adapter set name
            std::getline(ss, name, '|');

            // Field 2: start side (name,sequence)
            std::getline(ss, start_field, '|');

            // Field 3: end side (name,sequence) — may be empty if line ends
            // with '|'
            std::getline(ss, end_field);

            // Trim whitespace from each field
            name = trim(name);
            start_field = trim(start_field);
            end_field = trim(end_field);

            // Parse start side
            std::string start_name, start_seq;
            if (!start_field.empty()) {
                auto comma = start_field.find(',');
                if (comma == std::string::npos) {
                    std::cerr
                        << "Error: malformed line " << line_num
                        << " in adapter file (missing comma in start field)\n";
                    continue;
                }
                start_name = trim(start_field.substr(0, comma));
                start_seq = trim(start_field.substr(comma + 1));
                if (start_name.empty() || start_seq.empty()) {
                    std::cerr
                        << "Error: malformed line " << line_num
                        << " in adapter file (empty start name or sequence)\n";
                    continue;
                }
            }

            // Parse end side
            std::string end_name, end_seq;
            if (!end_field.empty()) {
                auto comma = end_field.find(',');
                if (comma == std::string::npos) {
                    std::cerr
                        << "Error: malformed line " << line_num
                        << " in adapter file (missing comma in end field)\n";
                    continue;
                }
                end_name = trim(end_field.substr(0, comma));
                end_seq = trim(end_field.substr(comma + 1));
                if (end_name.empty() || end_seq.empty()) {
                    std::cerr
                        << "Error: malformed line " << line_num
                        << " in adapter file (empty end name or sequence)\n";
                    continue;
                }
            }

            // At least one side must be present
            if (start_field.empty() && end_field.empty()) {
                std::cerr << "Error: malformed line " << line_num
                          << " in adapter file (no start or end sequences)\n";
                continue;
            }

            result.push_back(AdapterSet::create(name, start_name, start_seq,
                                                end_name, end_seq));
        }

        return result;
    }

    // =================================================================================================
    // AdapterRegistry
    // =================================================================================================

    AdapterRegistry AdapterRegistry::get_builtin() {
        return AdapterRegistry(get_builtin_adapters());
    }

    void AdapterRegistry::add(const AdapterSet& adapter) {
        adapters_.push_back(adapter);
    }

    void AdapterRegistry::add_file(const std::string& path) {
        std::vector<AdapterSet> file_adapters = load_adapter_file(path);
        adapters_ = merge_adapter_sets(adapters_, file_adapters);
    }

    std::vector<AdapterSet> AdapterRegistry::get_all() const {
        return adapters_;
    }

    std::vector<AdapterSet> AdapterRegistry::get_search() const {
        std::vector<AdapterSet> search;
        search.reserve(adapters_.size());
        std::copy_if(adapters_.begin(), adapters_.end(),
                     std::back_inserter(search), [](const AdapterSet& a) {
                         return a.name.find("(full sequence)") ==
                                std::string::npos;
                     });
        return search;
    }

    bool AdapterRegistry::empty() const { return adapters_.empty(); }

    std::size_t AdapterRegistry::size() const { return adapters_.size(); }

}  // namespace porechop
