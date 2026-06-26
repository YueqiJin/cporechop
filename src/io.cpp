/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include "porechop/io.hpp"

#include <zlib.h>

#include <array>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace porechop {

    namespace {

        /// Strip trailing CR (\r) and LF (\n) characters from a string.
        /// std::getline strips the delimiter (\n) but leaves \r on Windows line
        /// endings (\r\n).  This helper removes trailing \r to normalise input.
        void trim_trailing_cr(std::string& line) {
            while (!line.empty() &&
                   (line.back() == '\r' || line.back() == '\n')) {
                line.pop_back();
            }
        }

        bool is_gzip_path(const std::string& path) {
            constexpr std::string_view suffix = ".gz";
            if (path.size() < suffix.size()) {
                return false;
            }

            const std::string_view tail{
                path.data() + path.size() - suffix.size(), suffix.size()};
            for (std::size_t i = 0; i < suffix.size(); ++i) {
                const auto lhs = static_cast<unsigned char>(tail[i]);
                const auto rhs = static_cast<unsigned char>(suffix[i]);
                if (std::tolower(lhs) != std::tolower(rhs)) {
                    return false;
                }
            }
            return true;
        }

        std::string gz_error_message(gzFile file, const std::string& path) {
            int errnum = Z_OK;
            const char* message = gzerror(file, &errnum);
            if (errnum == Z_ERRNO) {
                return "I/O error while reading gzip file: " + path;
            }
            if (message != nullptr) {
                return "Error while reading gzip file " + path + ": " + message;
            }
            return "Error while reading gzip file: " + path;
        }

        std::string gz_decompress(const std::string& path) {
            gzFile file = gzopen(path.c_str(), "rb");
            if (file == nullptr) {
                throw std::runtime_error("Could not open gzip file: " + path);
            }

            std::string decompressed;
            std::array<char, 128 * 1024> buffer{};

            while (true) {
                const int bytes_read =
                    gzread(file, buffer.data(),
                           static_cast<unsigned int>(buffer.size()));
                if (bytes_read < 0) {
                    const std::string message = gz_error_message(file, path);
                    gzclose(file);
                    throw std::runtime_error(message);
                }
                if (bytes_read == 0) {
                    break;
                }
                decompressed.append(buffer.data(),
                                    static_cast<std::size_t>(bytes_read));
            }

            const int close_status = gzclose(file);
            if (close_status != Z_OK) {
                throw std::runtime_error(
                    "Error while closing gzip file (possibly "
                    "truncated/corrupt): " +
                    path);
            }
            if (decompressed.empty()) {
                throw std::runtime_error("Gzip file is empty: " + path);
            }

            return decompressed;
        }

        char gz_read_first_char(const std::string& path) {
            gzFile file = gzopen(path.c_str(), "rb");
            if (file == nullptr) {
                throw std::runtime_error("Could not open gzip file: " + path);
            }

            char first_char = 0;
            const int bytes_read = gzread(file, &first_char, 1);
            if (bytes_read < 0) {
                const std::string message = gz_error_message(file, path);
                gzclose(file);
                throw std::runtime_error(message);
            }

            const int close_status = gzclose(file);
            if (close_status != Z_OK) {
                throw std::runtime_error(
                    "Error while closing gzip file (possibly "
                    "truncated/corrupt): " +
                    path);
            }
            if (bytes_read == 0) {
                throw std::runtime_error("Gzip file is empty: " + path);
            }

            return first_char;
        }

        std::vector<Read> load_fasta_stream(std::istream& stream) {
            std::vector<Read> reads;
            std::string line;
            std::string name;
            std::string sequence;

            while (std::getline(stream, line)) {
                trim_trailing_cr(line);
                if (line.empty()) {
                    continue;
                }

                if (line[0] == '>') {
                    // Save previous record
                    if (!name.empty()) {
                        reads.emplace_back(name, sequence, "");
                        sequence.clear();
                    }
                    name = line.substr(1);  // full header without '>'
                } else {
                    sequence += line;
                }
            }

            // Save last record
            if (!name.empty()) {
                reads.emplace_back(name, sequence, "");
            }

            return reads;
        }

        std::vector<Read> load_fastq_stream(std::istream& stream) {
            std::vector<Read> reads;
            std::string line;

            while (std::getline(stream, line)) {
                trim_trailing_cr(line);

                // Skip leading empty lines before '@'
                if (line.empty()) {
                    continue;
                }

                if (line[0] != '@') {
                    throw std::runtime_error(
                        "Expected '@' at start of FASTQ record, got: " +
                        (line.empty() ? std::string("(empty)")
                                      : line.substr(0, 50)));
                }

                std::string full_name = line.substr(1);  // header without '@'

                // Read sequence line
                if (!std::getline(stream, line)) {
                    throw std::runtime_error(
                        "Unexpected end of FASTQ file while reading sequence "
                        "for: " +
                        full_name);
                }
                trim_trailing_cr(line);
                std::string sequence = line;

                // Read spacer line (typically '+')
                if (!std::getline(stream, line)) {
                    throw std::runtime_error(
                        "Unexpected end of FASTQ file while reading spacer "
                        "for: " +
                        full_name);
                }
                // spacer is ignored, but we consume the line
                trim_trailing_cr(line);

                // Read quality line
                if (!std::getline(stream, line)) {
                    throw std::runtime_error(
                        "Unexpected end of FASTQ file while reading qualities "
                        "for: " +
                        full_name);
                }
                trim_trailing_cr(line);
                std::string qualities = line;

                reads.emplace_back(full_name, sequence, qualities);
            }

            return reads;
        }

    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // load_fasta
    // ---------------------------------------------------------------------------
    std::vector<Read> load_fasta(const std::string& path) {
        if (is_gzip_path(path)) {
            const std::string decompressed = gz_decompress(path);
            std::istringstream stream(decompressed);
            return load_fasta_stream(stream);
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open FASTA file: " + path);
        }

        return load_fasta_stream(file);
    }

    // ---------------------------------------------------------------------------
    // load_fastq
    // ---------------------------------------------------------------------------
    std::vector<Read> load_fastq(const std::string& path) {
        if (is_gzip_path(path)) {
            const std::string decompressed = gz_decompress(path);
            std::istringstream stream(decompressed);
            return load_fastq_stream(stream);
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open FASTQ file: " + path);
        }

        return load_fastq_stream(file);
    }

    // ---------------------------------------------------------------------------
    // detect_read_type
    // ---------------------------------------------------------------------------
    std::string detect_read_type(const std::string& path) {
        char first_char = 0;

        if (is_gzip_path(path)) {
            first_char = gz_read_first_char(path);
        } else {
            std::ifstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open file: " + path);
            }

            file.get(first_char);
            if (!file) {
                throw std::runtime_error("File is empty: " + path);
            }
        }

        if (first_char == '>') {
            return "FASTA";
        }
        if (first_char == '@') {
            return "FASTQ";
        }
        throw std::runtime_error("File is neither FASTA nor FASTQ: " + path);
    }

    // ---------------------------------------------------------------------------
    // load_reads
    // ---------------------------------------------------------------------------
    std::vector<Read> load_reads(const std::string& path) {
        const std::string type = detect_read_type(path);
        if (type == "FASTA") {
            return load_fasta(path);
        }
        // type must be "FASTQ" (detect_read_type throws otherwise)
        return load_fastq(path);
    }

    // ---------------------------------------------------------------------------
    // write_fasta
    // ---------------------------------------------------------------------------
    void write_fasta(const std::vector<Read>& reads, std::ostream& os) {
        for (const auto& read : reads) {
            os << read.get_fasta(0, false, true);
        }
    }

    // ---------------------------------------------------------------------------
    // write_fastq
    // ---------------------------------------------------------------------------
    void write_fastq(const std::vector<Read>& reads, std::ostream& os) {
        for (const auto& read : reads) {
            os << read.get_fastq(0, false, true);
        }
    }

}  // namespace porechop
