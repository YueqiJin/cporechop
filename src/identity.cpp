/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/identity.hpp"

#include <algorithm>
#include <string>

namespace porechop {

    namespace {

        char alignment_char_at(std::string_view alignment, std::size_t index) {
            return index < alignment.size() ? alignment[index] : '-';
        }

    }  // namespace

    AdapterAlignment compute_identity_from_alignment_strings(
        std::string_view read_alignment, std::string_view adapter_alignment,
        int raw_score) {
        AdapterAlignment result;
        result.raw_score = raw_score;

        const int alignment_length = static_cast<int>(
            std::max(read_alignment.size(), adapter_alignment.size()));
        if (alignment_length == 0) {
            return result;
        }

        int alignment_start_pos = -1;
        bool read_started = false;
        bool adapter_started = false;
        for (int i = 0; i < alignment_length; ++i) {
            if (alignment_char_at(read_alignment,
                                  static_cast<std::size_t>(i)) != '-') {
                read_started = true;
            }
            if (alignment_char_at(adapter_alignment,
                                  static_cast<std::size_t>(i)) != '-') {
                adapter_started = true;
            }
            if (read_started && adapter_started) {
                alignment_start_pos = i;
                break;
            }
        }

        int alignment_end_pos = -1;
        bool read_ended = false;
        bool adapter_ended = false;
        for (int i = alignment_length - 1; i >= 0; --i) {
            if (alignment_char_at(read_alignment,
                                  static_cast<std::size_t>(i)) != '-') {
                read_ended = true;
            }
            if (alignment_char_at(adapter_alignment,
                                  static_cast<std::size_t>(i)) != '-') {
                adapter_ended = true;
            }
            if (read_ended && adapter_ended) {
                alignment_end_pos = i;
                break;
            }
        }

        if (alignment_start_pos == -1 || alignment_end_pos == -1) {
            return result;
        }

        int adapter_alignment_start_pos = -1;
        for (int i = 0; i < alignment_length; ++i) {
            if (alignment_char_at(adapter_alignment,
                                  static_cast<std::size_t>(i)) != '-') {
                adapter_alignment_start_pos = i;
                break;
            }
        }

        int adapter_alignment_end_pos = -1;
        for (int i = alignment_length - 1; i >= 0; --i) {
            if (alignment_char_at(adapter_alignment,
                                  static_cast<std::size_t>(i)) != '-') {
                adapter_alignment_end_pos = i;
                break;
            }
        }

        int aligned_match_count = 0;
        for (int i = alignment_start_pos; i <= alignment_end_pos; ++i) {
            if (alignment_char_at(adapter_alignment,
                                  static_cast<std::size_t>(i)) ==
                alignment_char_at(read_alignment,
                                  static_cast<std::size_t>(i))) {
                ++aligned_match_count;
            }
        }
        const int aligned_region_length =
            alignment_end_pos - alignment_start_pos + 1;
        result.aligned_region_percent_id =
            100.0 * aligned_match_count / aligned_region_length;

        int full_adapter_match_count = 0;
        for (int i = adapter_alignment_start_pos;
             i <= adapter_alignment_end_pos; ++i) {
            if (alignment_char_at(adapter_alignment,
                                  static_cast<std::size_t>(i)) ==
                alignment_char_at(read_alignment,
                                  static_cast<std::size_t>(i))) {
                ++full_adapter_match_count;
            }
        }
        const int full_adapter_length =
            adapter_alignment_end_pos - adapter_alignment_start_pos + 1;
        result.full_adapter_percent_id =
            100.0 * full_adapter_match_count / full_adapter_length;

        int read_bases = 0;
        int adapter_bases = 0;
        for (int i = 0; i < alignment_length; ++i) {
            const char read_base =
                alignment_char_at(read_alignment, static_cast<std::size_t>(i));
            const char adapter_base = alignment_char_at(
                adapter_alignment, static_cast<std::size_t>(i));

            if (i == alignment_start_pos) {
                result.read_begin_pos = read_bases;
                result.adapter_begin_pos = adapter_bases;
            }
            if (i == alignment_end_pos) {
                result.read_end_pos = read_bases;
                result.adapter_end_pos = adapter_bases;
            }

            if (read_base != '-') {
                ++read_bases;
            }
            if (adapter_base != '-') {
                ++adapter_bases;
            }
        }

        return result;
    }

}  // namespace porechop
