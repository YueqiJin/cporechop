#pragma once

/*
 * Copyright 2026 Yueqi Jin (jinyueqi@pku.edu.cn)
 * https://github.com/YueqiJin/cporechop
 *
 * This project is a derivative of Porechop (https://github.com/rrwick/Porechop)
 * by Ryan Wick, adapted for C++20 with seqan3.
 */

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include "porechop/types.hpp"

namespace porechop {

    // =================================================================================================
    // get_builtin_adapters: returns all 119 built-in adapter/barcode sets
    // matching the original list. Order: SQK-NSK007, Rapid, RBK004_upstream,
    // SQK-MAP006, SQK-MAP006 short, PCR adapters 1-3,
    //        1D^2 part 1-2, cDNA SSP, Barcode 1-12 (reverse), Barcode 1-96
    //        (forward).
    // =================================================================================================
    inline std::vector<AdapterSet> get_builtin_adapters() {
        std::vector<AdapterSet> adapters;

        // ---- Non-barcode adapters (11) ----

        adapters.push_back(AdapterSet::create(
            "SQK-NSK007", "SQK-NSK007_Y_Top", "AATGTACTTCGTTCAGTTACGTATTGCT",
            "SQK-NSK007_Y_Bottom", "GCAATACGTAACTGAACGAAGT"));

        adapters.push_back(AdapterSet::create(
            "Rapid", "Rapid_adapter",
            "GTTTTCGCATTTATCGTGAAACGCTTTCGCGTTTTTCGTGCGCCGCTTCA", "", ""));

        adapters.push_back(AdapterSet::create(
            "RBK004_upstream", "RBK004_upstream",
            "AATGTACTTCGTTCAGTTACGGCTTGGGTGTTTAACC", "", ""));

        adapters.push_back(AdapterSet::create(
            "SQK-MAP006", "SQK-MAP006_Y_Top_SK63",
            "GGTTGTTTCTGTTGGTGCTGATATTGCT", "SQK-MAP006_Y_Bottom_SK64",
            "GCAATATCAGCACCAACAGAAA"));

        adapters.push_back(AdapterSet::create(
            "SQK-MAP006 short", "SQK-MAP006_Short_Y_Top_LI32",
            "CGGCGTCTGCTTGGGTGTTTAACCT", "SQK-MAP006_Short_Y_Bottom_LI33",
            "GGTTAAACACCCAAGCAGACGCCG"));

        adapters.push_back(AdapterSet::create(
            "PCR adapters 1", "PCR_1_start", "ACTTGCCTGTCGCTCTATCTTC",
            "PCR_1_end", "GAAGATAGAGCGACAGGCAAGT"));

        adapters.push_back(AdapterSet::create(
            "PCR adapters 2", "PCR_2_start", "TTTCTGTTGGTGCTGATATTGC",
            "PCR_2_end", "GCAATATCAGCACCAACAGAAA"));

        adapters.push_back(AdapterSet::create(
            "PCR adapters 3", "PCR_3_start", "TACTTGCCTGTCGCTCTATCTTC",
            "PCR_3_end", "GAAGATAGAGCGACAGGCAAGTA"));

        adapters.push_back(AdapterSet::create(
            "1D^2 part 1", "1D2_part_1_start", "GAGAGGTTCCAAGTCAGAGAGGTTCCT",
            "1D2_part_1_end", "AGGAACCTCTCTGACTTGGAACCTCTC"));

        adapters.push_back(AdapterSet::create(
            "1D^2 part 2", "1D2_part_2_start",
            "CTTCGTTCAGTTACGTATTGCTGGCGTCTGCTT", "1D2_part_2_end",
            "CACCCAAGCAGACGCCAGCAATACGTAACT"));

        adapters.push_back(AdapterSet::create(
            "cDNA SSP", "cDNA_SSP", "TTTCTGTTGGTGCTGATATTGCTGCCATTACGGCCGGG",
            "cDNA_SSP_rev", "CCCGGCCGTAATGGCAGCAATATCAGCACCAACAGAAA"));

        // ---- Barcode 1-12 (reverse) ----

        adapters.push_back(AdapterSet::create(
            "Barcode 1 (reverse)", "BC01_rev", "CACAAAGACACCGACAACTTTCTT",
            "BC01", "AAGAAAGTTGTCGGTGTCTTTGTG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 2 (reverse)", "BC02_rev", "ACAGACGACTACAAACGGAATCGA",
            "BC02", "TCGATTCCGTTTGTAGTCGTCTGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 3 (reverse)", "BC03_rev", "CCTGGTAACTGGGACACAAGACTC",
            "BC03", "GAGTCTTGTGTCCCAGTTACCAGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 4 (reverse)", "BC04_rev", "TAGGGAAACACGATAGAATCCGAA",
            "BC04", "TTCGGATTCTATCGTGTTTCCCTA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 5 (reverse)", "BC05_rev", "AAGGTTACACAAACCCTGGACAAG",
            "BC05", "CTTGTCCAGGGTTTGTGTAACCTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 6 (reverse)", "BC06_rev", "GACTACTTTCTGCCTTTGCGAGAA",
            "BC06", "TTCTCGCAAAGGCAGAAAGTAGTC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 7 (reverse)", "BC07_rev", "AAGGATTCATTCCCACGGTAACAC",
            "BC07", "GTGTTACCGTGGGAATGAATCCTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 8 (reverse)", "BC08_rev", "ACGTAACTTGGTTTGTTCCCTGAA",
            "BC08", "TTCAGGGAACAAACCAAGTTACGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 9 (reverse)", "BC09_rev", "AACCAAGACTCGCTGTGCCTAGTT",
            "BC09", "AACTAGGCACAGCGAGTCTTGGTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 10 (reverse)", "BC10_rev", "GAGAGGACAAAGGTTTCAACGCTT",
            "BC10", "AAGCGTTGAAACCTTTGTCCTCTC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 11 (reverse)", "BC11_rev", "TCCATTCCCTCCGATAGATGAAAC",
            "BC11", "GTTTCATCTATCGGAGGGAATGGA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 12 (reverse)", "BC12_rev", "TCCGATTCTGCTTCTTTCTACCTG",
            "BC12", "CAGGTAGAAAGAAGCAGAATCGGA"));

        // ---- Barcode 1-96 (forward) ----

        adapters.push_back(AdapterSet::create(
            "Barcode 1 (forward)", "BC01", "AAGAAAGTTGTCGGTGTCTTTGTG",
            "BC01_rev", "CACAAAGACACCGACAACTTTCTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 2 (forward)", "BC02", "TCGATTCCGTTTGTAGTCGTCTGT",
            "BC02_rev", "ACAGACGACTACAAACGGAATCGA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 3 (forward)", "BC03", "GAGTCTTGTGTCCCAGTTACCAGG",
            "BC03_rev", "CCTGGTAACTGGGACACAAGACTC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 4 (forward)", "BC04", "TTCGGATTCTATCGTGTTTCCCTA",
            "BC04_rev", "TAGGGAAACACGATAGAATCCGAA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 5 (forward)", "BC05", "CTTGTCCAGGGTTTGTGTAACCTT",
            "BC05_rev", "AAGGTTACACAAACCCTGGACAAG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 6 (forward)", "BC06", "TTCTCGCAAAGGCAGAAAGTAGTC",
            "BC06_rev", "GACTACTTTCTGCCTTTGCGAGAA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 7 (forward)", "BC07", "GTGTTACCGTGGGAATGAATCCTT",
            "BC07_rev", "AAGGATTCATTCCCACGGTAACAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 8 (forward)", "BC08", "TTCAGGGAACAAACCAAGTTACGT",
            "BC08_rev", "ACGTAACTTGGTTTGTTCCCTGAA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 9 (forward)", "BC09", "AACTAGGCACAGCGAGTCTTGGTT",
            "BC09_rev", "AACCAAGACTCGCTGTGCCTAGTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 10 (forward)", "BC10", "AAGCGTTGAAACCTTTGTCCTCTC",
            "BC10_rev", "GAGAGGACAAAGGTTTCAACGCTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 11 (forward)", "BC11", "GTTTCATCTATCGGAGGGAATGGA",
            "BC11_rev", "TCCATTCCCTCCGATAGATGAAAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 12 (forward)", "BC12", "CAGGTAGAAAGAAGCAGAATCGGA",
            "BC12_rev", "TCCGATTCTGCTTCTTTCTACCTG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 13 (forward)", "BC13", "AGAACGACTTCCATACTCGTGTGA",
            "BC13_rev", "TCACACGAGTATGGAAGTCGTTCT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 14 (forward)", "BC14", "AACGAGTCTCTTGGGACCCATAGA",
            "BC14_rev", "TCTATGGGTCCCAAGAGACTCGTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 15 (forward)", "BC15", "AGGTCTACCTCGCTAACACCACTG",
            "BC15_rev", "CAGTGGTGTTAGCGAGGTAGACCT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 16 (forward)", "BC16", "CGTCAACTGACAGTGGTTCGTACT",
            "BC16_rev", "AGTACGAACCACTGTCAGTTGACG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 17 (forward)", "BC17", "ACCCTCCAGGAAAGTACCTCTGAT",
            "BC17_rev", "ATCAGAGGTACTTTCCTGGAGGGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 18 (forward)", "BC18", "CCAAACCCAACAACCTAGATAGGC",
            "BC18_rev", "GCCTATCTAGGTTGTTGGGTTTGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 19 (forward)", "BC19", "GTTCCTCGTGCAGTGTCAAGAGAT",
            "BC19_rev", "ATCTCTTGACACTGCACGAGGAAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 20 (forward)", "BC20", "TTGCGTCCTGTTACGAGAACTCAT",
            "BC20_rev", "ATGAGTTCTCGTAACAGGACGCAA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 21 (forward)", "BC21", "GAGCCTCTCATTGTCCGTTCTCTA",
            "BC21_rev", "TAGAGAACGGACAATGAGAGGCTC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 22 (forward)", "BC22", "ACCACTGCCATGTATCAAAGTACG",
            "BC22_rev", "CGTACTTTGATACATGGCAGTGGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 23 (forward)", "BC23", "CTTACTACCCAGTGAACCTCCTCG",
            "BC23_rev", "CGAGGAGGTTCACTGGGTAGTAAG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 24 (forward)", "BC24", "GCATAGTTCTGCATGATGGGTTAG",
            "BC24_rev", "CTAACCCATCATGCAGAACTATGC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 25 (forward)", "BC25", "GTAAGTTGGGTATGCAACGCAATG",
            "BC25_rev", "CATTGCGTTGCATACCCAACTTAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 26 (forward)", "BC26", "CATACAGCGACTACGCATTCTCAT",
            "BC26_rev", "ATGAGAATGCGTAGTCGCTGTATG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 27 (forward)", "BC27", "CGACGGTTAGATTCACCTCTTACA",
            "BC27_rev", "TGTAAGAGGTGAATCTAACCGTCG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 28 (forward)", "BC28", "TGAAACCTAAGAAGGCACCGTATC",
            "BC28_rev", "GATACGGTGCCTTCTTAGGTTTCA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 29 (forward)", "BC29", "CTAGACACCTTGGGTTGACAGACC",
            "BC29_rev", "GGTCTGTCAACCCAAGGTGTCTAG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 30 (forward)", "BC30", "TCAGTGAGGATCTACTTCGACCCA",
            "BC30_rev", "TGGGTCGAAGTAGATCCTCACTGA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 31 (forward)", "BC31", "TGCGTACAGCAATCAGTTACATTG",
            "BC31_rev", "CAATGTAACTGATTGCTGTACGCA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 32 (forward)", "BC32", "CCAGTAGAAGTCCGACAACGTCAT",
            "BC32_rev", "ATGACGTTGTCGGACTTCTACTGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 33 (forward)", "BC33", "CAGACTTGGTACGGTTGGGTAACT",
            "BC33_rev", "AGTTACCCAACCGTACCAAGTCTG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 34 (forward)", "BC34", "GGACGAAGAACTCAAGTCAAAGGC",
            "BC34_rev", "GCCTTTGACTTGAGTTCTTCGTCC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 35 (forward)", "BC35", "CTACTTACGAAGCTGAGGGACTGC",
            "BC35_rev", "GCAGTCCCTCAGCTTCGTAAGTAG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 36 (forward)", "BC36", "ATGTCCCAGTTAGAGGAGGAAACA",
            "BC36_rev", "TGTTTCCTCCTCTAACTGGGACAT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 37 (forward)", "BC37", "GCTTGCGATTGATGCTTAGTATCA",
            "BC37_rev", "TGATACTAAGCATCAATCGCAAGC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 38 (forward)", "BC38", "ACCACAGGAGGACGATACAGAGAA",
            "BC38_rev", "TTCTCTGTATCGTCCTCCTGTGGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 39 (forward)", "BC39", "CCACAGTGTCAACTAGAGCCTCTC",
            "BC39_rev", "GAGAGGCTCTAGTTGACACTGTGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 40 (forward)", "BC40", "TAGTTTGGATGACCAAGGATAGCC",
            "BC40_rev", "GGCTATCCTTGGTCATCCAAACTA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 41 (forward)", "BC41", "GGAGTTCGTCCAGAGAAGTACACG",
            "BC41_rev", "CGTGTACTTCTCTGGACGAACTCC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 42 (forward)", "BC42", "CTACGTGTAAGGCATACCTGCCAG",
            "BC42_rev", "CTGGCAGGTATGCCTTACACGTAG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 43 (forward)", "BC43", "CTTTCGTTGTTGACTCGACGGTAG",
            "BC43_rev", "CTACCGTCGAGTCAACAACGAAAG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 44 (forward)", "BC44", "AGTAGAAAGGGTTCCTTCCCACTC",
            "BC44_rev", "GAGTGGGAAGGAACCCTTTCTACT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 45 (forward)", "BC45", "GATCCAACAGAGATGCCTTCAGTG",
            "BC45_rev", "CACTGAAGGCATCTCTGTTGGATC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 46 (forward)", "BC46", "GCTGTGTTCCACTTCATTCTCCTG",
            "BC46_rev", "CAGGAGAATGAAGTGGAACACAGC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 47 (forward)", "BC47", "GTGCAACTTTCCCACAGGTAGTTC",
            "BC47_rev", "GAACTACCTGTGGGAAAGTTGCAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 48 (forward)", "BC48", "CATCTGGAACGTGGTACACCTGTA",
            "BC48_rev", "TACAGGTGTACCACGTTCCAGATG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 49 (forward)", "BC49", "ACTGGTGCAGCTTTGAACATCTAG",
            "BC49_rev", "CTAGATGTTCAAAGCTGCACCAGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 50 (forward)", "BC50", "ATGGACTTTGGTAACTTCCTGCGT",
            "BC50_rev", "ACGCAGGAAGTTACCAAAGTCCAT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 51 (forward)", "BC51", "GTTGAATGAGCCTACTGGGTCCTC",
            "BC51_rev", "GAGGACCCAGTAGGCTCATTCAAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 52 (forward)", "BC52", "TGAGAGACAAGATTGTTCGTGGAC",
            "BC52_rev", "GTCCACGAACAATCTTGTCTCTCA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 53 (forward)", "BC53", "AGATTCAGACCGTCTCATGCAAAG",
            "BC53_rev", "CTTTGCATGAGACGGTCTGAATCT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 54 (forward)", "BC54", "CAAGAGCTTTGACTAAGGAGCATG",
            "BC54_rev", "CATGCTCCTTAGTCAAAGCTCTTG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 55 (forward)", "BC55", "TGGAAGATGAGACCCTGATCTACG",
            "BC55_rev", "CGTAGATCAGGGTCTCATCTTCCA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 56 (forward)", "BC56", "TCACTACTCAACAGGTGGCATGAA",
            "BC56_rev", "TTCATGCCACCTGTTGAGTAGTGA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 57 (forward)", "BC57", "GCTAGGTCAATCTCCTTCGGAAGT",
            "BC57_rev", "ACTTCCGAAGGAGATTGACCTAGC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 58 (forward)", "BC58", "CAGGTTACTCCTCCGTGAGTCTGA",
            "BC58_rev", "TCAGACTCACGGAGGAGTAACCTG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 59 (forward)", "BC59", "TCAATCAAGAAGGGAAAGCAAGGT",
            "BC59_rev", "ACCTTGCTTTCCCTTCTTGATTGA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 60 (forward)", "BC60", "CATGTTCAACCAAGGCTTCTATGG",
            "BC60_rev", "CCATAGAAGCCTTGGTTGAACATG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 61 (forward)", "BC61", "AGAGGGTACTATGTGCCTCAGCAC",
            "BC61_rev", "GTGCTGAGGCACATAGTACCCTCT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 62 (forward)", "BC62", "CACCCACACTTACTTCAGGACGTA",
            "BC62_rev", "TACGTCCTGAAGTAAGTGTGGGTG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 63 (forward)", "BC63", "TTCTGAAGTTCCTGGGTCTTGAAC",
            "BC63_rev", "GTTCAAGACCCAGGAACTTCAGAA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 64 (forward)", "BC64", "GACAGACACCGTTCATCGACTTTC",
            "BC64_rev", "GAAAGTCGATGAACGGTGTCTGTC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 65 (forward)", "BC65", "TTCTCAGTCTTCCTCCAGACAAGG",
            "BC65_rev", "CCTTGTCTGGAGGAAGACTGAGAA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 66 (forward)", "BC66", "CCGATCCTTGTGGCTTCTAACTTC",
            "BC66_rev", "GAAGTTAGAAGCCACAAGGATCGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 67 (forward)", "BC67", "GTTTGTCATACTCGTGTGCTCACC",
            "BC67_rev", "GGTGAGCACACGAGTATGACAAAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 68 (forward)", "BC68", "GAATCTAAGCAAACACGAAGGTGG",
            "BC68_rev", "CCACCTTCGTGTTTGCTTAGATTC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 69 (forward)", "BC69", "TACAGTCCGAGCCTCATGTGATCT",
            "BC69_rev", "AGATCACATGAGGCTCGGACTGTA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 70 (forward)", "BC70", "ACCGAGATCCTACGAATGGAGTGT",
            "BC70_rev", "ACACTCCATTCGTAGGATCTCGGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 71 (forward)", "BC71", "CCTGGGAGCATCAGGTAGTAACAG",
            "BC71_rev", "CTGTTACTACCTGATGCTCCCAGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 72 (forward)", "BC72", "TAGCTGACTGTCTTCCATACCGAC",
            "BC72_rev", "GTCGGTATGGAAGACAGTCAGCTA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 73 (forward)", "BC73", "AAGAAACAGGATGACAGAACCCTC",
            "BC73_rev", "GAGGGTTCTGTCATCCTGTTTCTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 74 (forward)", "BC74", "TACAAGCATCCCAACACTTCCACT",
            "BC74_rev", "AGTGGAAGTGTTGGGATGCTTGTA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 75 (forward)", "BC75", "GACCATTGTGATGAACCCTGTTGT",
            "BC75_rev", "ACAACAGGGTTCATCACAATGGTC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 76 (forward)", "BC76", "ATGCTTGTTACATCAACCCTGGAC",
            "BC76_rev", "GTCCAGGGTTGATGTAACAAGCAT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 77 (forward)", "BC77", "CGACCTGTTTCTCAGGGATACAAC",
            "BC77_rev", "GTTGTATCCCTGAGAAACAGGTCG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 78 (forward)", "BC78", "AACAACCGAACCTTTGAATCAGAA",
            "BC78_rev", "TTCTGATTCAAAGGTTCGGTTGTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 79 (forward)", "BC79", "TCTCGGAGATAGTTCTCACTGCTG",
            "BC79_rev", "CAGCAGTGAGAACTATCTCCGAGA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 80 (forward)", "BC80", "CGGATGAACATAGGATAGCGATTC",
            "BC80_rev", "GAATCGCTATCCTATGTTCATCCG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 81 (forward)", "BC81", "CCTCATCTTGTGAAGTTGTTTCGG",
            "BC81_rev", "CCGAAACAACTTCACAAGATGAGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 82 (forward)", "BC82", "ACGGTATGTCGAGTTCCAGGACTA",
            "BC82_rev", "TAGTCCTGGAACTCGACATACCGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 83 (forward)", "BC83", "TGGCTTGATCTAGGTAAGGTCGAA",
            "BC83_rev", "TTCGACCTTACCTAGATCAAGCCA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 84 (forward)", "BC84", "GTAGTGGACCTAGAACCTGTGCCA",
            "BC84_rev", "TGGCACAGGTTCTAGGTCCACTAC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 85 (forward)", "BC85", "AACGGAGGAGTTAGTTGGATGATC",
            "BC85_rev", "GATCATCCAACTAACTCCTCCGTT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 86 (forward)", "BC86", "AGGTGATCCCAACAAGCGTAAGTA",
            "BC86_rev", "TACTTACGCTTGTTGGGATCACCT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 87 (forward)", "BC87", "TACATGCTCCTGTTGTTAGGGAGG",
            "BC87_rev", "CCTCCCTAACAACAGGAGCATGTA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 88 (forward)", "BC88", "TCTTCTACTACCGATCCGAAGCAG",
            "BC88_rev", "CTGCTTCGGATCGGTAGTAGAAGA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 89 (forward)", "BC89", "ACAGCATCAATGTTTGGCTAGTTG",
            "BC89_rev", "CAACTAGCCAAACATTGATGCTGT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 90 (forward)", "BC90", "GATGTAGAGGGTACGGTTTGAGGC",
            "BC90_rev", "GCCTCAAACCGTACCCTCTACATC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 91 (forward)", "BC91", "GGCTCCATAGGAACTCACGCTACT",
            "BC91_rev", "AGTAGCGTGAGTTCCTATGGAGCC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 92 (forward)", "BC92", "TTGTGAGTGGAAAGATACAGGACC",
            "BC92_rev", "GGTCCTGTATCTTTCCACTCACAA"));

        adapters.push_back(AdapterSet::create(
            "Barcode 93 (forward)", "BC93", "AGTTTCCATCACTTCAGACTTGGG",
            "BC93_rev", "CCCAAGTCTGAAGTGATGGAAACT"));

        adapters.push_back(AdapterSet::create(
            "Barcode 94 (forward)", "BC94", "GATTGTCCTCAAACTGCCACCTAC",
            "BC94_rev", "GTAGGTGGCAGTTTGAGGACAATC"));

        adapters.push_back(AdapterSet::create(
            "Barcode 95 (forward)", "BC95", "CCTGTCTGGAAGAAGAATGGACTT",
            "BC95_rev", "AAGTCCATTCTTCTTCCAGACAGG"));

        adapters.push_back(AdapterSet::create(
            "Barcode 96 (forward)", "BC96", "CTGAACGGTCATAGAGTCCACCAT",
            "BC96_rev", "ATGGTGGACTCTATGACCGTTCAG"));

        return adapters;
    }

    // =================================================================================================
    // get_search_adapters: returns only adapters whose name does NOT contain
    // "(full sequence)".
    // =================================================================================================
    inline std::vector<AdapterSet> get_search_adapters() {
        std::vector<AdapterSet> all = get_builtin_adapters();
        std::vector<AdapterSet> search;
        search.reserve(all.size());
        std::copy_if(all.begin(), all.end(), std::back_inserter(search),
                     [](const AdapterSet& a) {
                         return a.name.find("(full sequence)") ==
                                std::string::npos;
                     });
        return search;
    }

    // =================================================================================================
    // make_full_native_barcode_adapter: builds the full-length native barcode
    // adapter for barcode N. Uses the reverse barcode start/end sequences.
    // =================================================================================================
    inline AdapterSet make_full_native_barcode_adapter(
        int n, const std::string& start_barcode_seq,
        const std::string& end_barcode_seq) {
        std::string start_full_seq = "AATGTACTTCGTTCAGTTACGTATTGCTAAGGTTAA" +
                                     start_barcode_seq + "CAGCACCT";
        std::string end_full_seq =
            "AGGTGCTG" + end_barcode_seq + "TTAACCTTAGCAATACGTAACTGAACGAAGT";

        char num_buf[8];
        std::snprintf(num_buf, sizeof(num_buf), "%02d", n);

        return AdapterSet::create(
            "Native barcoding " + std::to_string(n) + " (full sequence)",
            "NB" + std::string(num_buf) + "_start", start_full_seq,
            "NB" + std::string(num_buf) + "_end", end_full_seq);
    }

    // =================================================================================================
    // make_old_full_rapid_barcode_adapter: builds the full-length old rapid
    // barcode adapter (SQK-RBK001). Uses the forward barcode start sequence.
    // =================================================================================================
    inline AdapterSet make_old_full_rapid_barcode_adapter(
        int n, const std::string& start_barcode_seq) {
        std::string start_full_seq =
            "AATGTACTTCGTTCAGTTACG" + std::string("TATTGCT") +
            start_barcode_seq +
            "GTTTTCGCATTTATCGTGAAACGCTTTCGCGTTTTTCGTGCGCCGCTTCA";

        char num_buf[8];
        std::snprintf(num_buf, sizeof(num_buf), "%02d", n);

        return AdapterSet::create(
            "Rapid barcoding " + std::to_string(n) + " (full sequence, old)",
            "RB" + std::string(num_buf) + "_full", start_full_seq, "", "");
    }

    // =================================================================================================
    // make_new_full_rapid_barcode_adapter: builds the full-length new rapid
    // barcode adapter (SQK-RBK004). Uses the forward barcode start sequence.
    // =================================================================================================
    inline AdapterSet make_new_full_rapid_barcode_adapter(
        int n, const std::string& start_barcode_seq) {
        std::string start_full_seq =
            "AATGTACTTCGTTCAGTTACG" + std::string("GCTTGGGTGTTTAACC") +
            start_barcode_seq +
            "GTTTTCGCATTTATCGTGAAACGCTTTCGCGTTTTTCGTGCGCCGCTTCA";

        char num_buf[8];
        std::snprintf(num_buf, sizeof(num_buf), "%02d", n);

        return AdapterSet::create(
            "Rapid barcoding " + std::to_string(n) + " (full sequence, new)",
            "RB" + std::string(num_buf) + "_full", start_full_seq, "", "");
    }

    // =================================================================================================
    // merge_adapter_sets: returns the union of two adapter vectors. Left-side
    // entries take priority when names conflict. Only entries from 'b' whose
    // names are not in 'a' are appended.
    // =================================================================================================
    inline std::vector<AdapterSet> merge_adapter_sets(
        const std::vector<AdapterSet>& a, const std::vector<AdapterSet>& b) {
        std::vector<AdapterSet> result = a;
        for (const auto& b_set : b) {
            bool found = false;
            for (const auto& a_set : result) {
                if (a_set.name == b_set.name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.push_back(b_set);
            }
        }
        return result;
    }

}  // namespace porechop
