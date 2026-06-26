/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/full_barcode.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "porechop/adapters.hpp"

namespace porechop {

    std::vector<AdapterSet> add_full_barcode_adapter_sets(
        const std::vector<AdapterSet>& matching_sets) {
        // Build a set of all adapter names for O(1) lookups
        std::unordered_set<std::string> names;
        names.reserve(matching_sets.size());
        for (const auto& as : matching_sets) {
            names.insert(as.name);
        }

        // Cache component presence checks
        bool has_nsk007 = names.find("SQK-NSK007") != names.end();
        bool has_rapid = names.find("Rapid") != names.end();
        bool has_rbk004_upstream = names.find("RBK004_upstream") != names.end();

        // Early return if no barcoding components at all
        if (!has_nsk007 && !has_rapid) {
            return matching_sets;
        }

        // Build name -> AdapterSet* map for extracting barcode sequences
        std::unordered_map<std::string, const AdapterSet*> name_to_as;
        name_to_as.reserve(matching_sets.size());
        for (const auto& as : matching_sets) {
            name_to_as[as.name] = &as;
        }

        // Result: preserve input, append full barcodes
        std::vector<AdapterSet> result = matching_sets;

        for (int i = 1; i <= 96; ++i) {
            std::string num_str = std::to_string(i);

            // ---- Native barcode full sequences ----
            if (has_nsk007) {
                std::string rev_name = "Barcode " + num_str + " (reverse)";
                auto it = name_to_as.find(rev_name);
                if (it != name_to_as.end()) {
                    const AdapterSet* rev_set = it->second;
                    result.push_back(make_full_native_barcode_adapter(
                        i, rev_set->start_sequence.sequence,
                        rev_set->end_sequence.sequence));
                }
            }

            // ---- Rapid barcode full sequences ----
            if (has_rapid) {
                std::string fwd_name = "Barcode " + num_str + " (forward)";
                auto it = name_to_as.find(fwd_name);
                if (it != name_to_as.end()) {
                    const AdapterSet* fwd_set = it->second;
                    if (has_rbk004_upstream) {
                        // New rapid barcode (SQK-RBK004)
                        result.push_back(make_new_full_rapid_barcode_adapter(
                            i, fwd_set->start_sequence.sequence));
                    } else if (has_nsk007) {
                        // Old rapid barcode (SQK-RBK001)
                        result.push_back(make_old_full_rapid_barcode_adapter(
                            i, fwd_set->start_sequence.sequence));
                    }
                }
            }
        }

        return result;
    }

}  // namespace porechop
