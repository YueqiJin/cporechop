# cporechop

A C++20 reimplementation of [Porechop](https://github.com/rrwick/Porechop), the tool for finding and removing adapters from Oxford Nanopore reads. This port uses **seqan3** instead of seqan2 and builds as both a shared library (`cporechop`) and a CLI executable (`cporechop`).

cporechop finds adapters on the ends of reads and trims them off. When a read has an adapter in its middle, it is treated as chimeric and split into separate reads. It also supports demultiplexing of Nanopore reads barcoded with the Native Barcoding Kit, PCR Barcoding Kit, or Rapid Barcoding Kit.

### Differences from original Porechop

cporechop uses **seqan3** for pairwise alignment, while the original Porechop uses **seqan2**. Although both engines use identical scoring parameters and semi-global alignment configuration, they are different implementations of the Smith-Waterman / Needleman-Wunsch DP algorithms and may trace different optimal paths through the alignment matrix for the same input. This can lead to small differences in:

- **Adapter detection scores** — the best-matching adapter set for a given read may differ by 1–2% identity, and a different read may produce the maximum score for a given adapter set.
- **Trim and split decisions** — end trimming and middle-adapter splitting counts typically differ by ~2–3% from original Porechop on real datasets.
- **Split-part boundaries** — when a read is split at a middle adapter, the exact cut point may shift by 1 bp.

These differences are inherent to the alignment engine swap and are not bugs. The output is functionally equivalent — the same adapters are detected, the same reads are trimmed, and the same chimeras are split.

---

## Requirements

- A C++20 compiler (GCC 11+, Clang 14+, or MSVC 2022+)
- CMake 3.20 or later
- Linux, macOS, or Windows

All dependencies (seqan3, CLI11) are fetched automatically via CMake's FetchContent. No system package installation is needed.

---

## Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets (library + executable)
cmake --build build

# Package for redistribution (TGZ on Linux/macOS, ZIP on Windows)
cd build && cpack
```

The TGZ/ZIP package includes shared libraries alongside the executable — it runs on other machines without installing dependencies.

The build produces two artifacts:

| Target | Path | Description |
|--------|------|-------------|
| `cporechop` | `lib/libcporechop.so` (or `lib/cporechop.dll` on Windows) | Shared library with public C++ API |
| `cporechop` | `bin/cporechop` (or `bin/Debug/cporechop.exe` on Windows) | CLI executable |

---

## Run

```bash
# Basic adapter trimming
cporechop -i input_reads.fastq.gz -o output_reads.fastq.gz

# Trimmed reads to stdout
cporechop -i input_reads.fastq.gz > output_reads.fastq

# Demultiplex barcoded reads
cporechop -i input_reads.fastq.gz -b output_dir

# Verbose output
cporechop -i input_reads.fastq.gz -o output_reads.fastq.gz --verbosity 2

# Use multiple threads
cporechop -i input_reads.fastq.gz -o output_reads.fastq.gz --threads 40

# Also works with FASTA
cporechop -i input_reads.fasta -o output_reads.fasta
```

---

## Full Usage

### Main options

| Flag | Argument | Default | Description |
|------|----------|---------|-------------|
| `-i`, `--input` | *path* | *(required)* | FASTA/FASTQ of input reads |
| `-o`, `--output` | *path* | stdout | Filename for trimmed reads (if not set, printed to stdout) |
| `--format` | `auto` \| `fasta` \| `fastq` \| `fasta.gz` \| `fastq.gz` | `auto` | Output format; `auto` infers from filename or input format |
| `-v`, `--verbosity` | `0`–`3` | `1` | Progress level: 0=none, 1=some, 2=lots, 3=full |
| `-t`, `--threads` | *int* | min(hw, 16) | Thread count for parallel adapter alignment |

### Adapter search settings

Control which adapter sets are detected as present in the sample.

| Flag | Argument | Default | Description |
|------|----------|---------|-------------|
| `--adapter_file` | *path* | — | Pipe-delimited file of custom adapters (merged with built-ins) |
| `--no_default_adapters` | *(flag)* | off | Skip built-in adapters, use only `--adapter_file` sets |
| `--adapter_threshold` | `0`–`100` | `90.0` | Min percent identity for an adapter set to be labelled present |
| `--check_reads` | *int* | `10000` | Reads aligned to all adapters to determine present sets |
| `--scoring_scheme` | *match,mismatch,gap_open,gap_extend* | `3,-6,-5,-2` | Alignment scoring parameters |

### End adapter settings

Control adapter trimming from read ends.

| Flag | Argument | Default | Description |
|------|----------|---------|-------------|
| `--end_size` | *int* | `150` | Base pairs searched at each read end |
| `--min_trim_size` | *int* | `4` | Minimum adapter alignment length to trigger a trim |
| `--extra_end_trim` | *int* | `2` | Extra bases removed past the adapter match |
| `--end_threshold` | `0`–`100` | `75.0` | Min percent identity for end adapter removal |

### Middle adapter settings

Control splitting of chimeric reads.

| Flag | Argument | Default | Description |
|------|----------|---------|-------------|
| `--no_split` | *(flag)* | off | Skip splitting reads based on middle adapters |
| `--discard_middle` | *(flag)* | off (auto-on with `-b`) | Discard reads with middle adapters instead of splitting |
| `--middle_threshold` | `0`–`100` | `90.0` | Min percent identity for middle adapter detection |
| `--extra_middle_trim_good_side` | *int* | `10` | Extra bases removed on the "good" side of a middle adapter |
| `--extra_middle_trim_bad_side` | *int* | `100` | Extra bases removed on the "bad" side of a middle adapter |
| `--min_split_read_size` | *int* | `1000` | Discard post-split pieces shorter than this (bp) |

### Barcode demultiplexing settings

Control barcode binning (requires `-b`).

| Flag | Argument | Default | Description |
|------|----------|---------|-------------|
| `-b`, `--barcode_dir` | *path* | — | Output directory for barcode-binned reads (incompatible with `-o`) |
| `--barcode_threshold` | `0`–`100` | `75.0` | Min percent identity for a read to be assigned to a barcode bin |
| `--barcode_diff` | *float* | `5.0` | Required identity gap between best and second-best barcode match |
| `--require_two_barcodes` | *(flag)* | off | Require barcode match on both start and end to assign |
| `--untrimmed` | *(flag)* | off | Bin reads without trimming (requires `-b`) |
| `--discard_unassigned` | *(flag)* | off | Discard reads that cannot be assigned to a barcode bin |

### Help

| Flag | Description |
|------|-------------|
| `-h`, `--help` | Show help message and exit |
| `--version` | Show version number and exit |

---

## How It Works

### Find matching adapter sets

cporechop first aligns a subset of reads (default 10000, change with `--check_reads`) to all known adapter sets. Adapter sets with at least one high identity match (default 90%, change with `--adapter_threshold`) are deemed present in the sample.

### Trim adapters from read ends

The first and last bases in each read (default 150, change with `--end_size`) are aligned to each present adapter set. When a long enough (default 4, change with `--min_trim_size`) and strong enough (default 75%, change with `--end_threshold`) match is found, the read is trimmed. A few extra bases (default 2, change with `--extra_end_trim`) past the adapter match are removed as well.

### Split reads with internal adapters

The entirety of each read is aligned to the present adapter sets to spot cases where an adapter is in the middle of the read, indicating a chimera. When a strong enough match is found (default 90%, change with `--middle_threshold`), the read is split. If the resulting parts are too short (default less than 1000 bp, change with `--min_split_read_size`), they are discarded.

### Barcode demultiplexing

cporechop looks for barcodes at the start and end of each read. All barcode matches are found, and if the best match is strong enough (default 75%, change with `--barcode_threshold`) and sufficiently better than the second-best match (default 5% better, change with `--barcode_diff`), the read is assigned to that barcode bin.

---

## Known Adapters

cporechop includes all known Nanopore adapter and barcode sequences from the original Porechop:

- Ligation kit adapters
- Rapid kit adapters
- PCR kit adapters
- Barcodes (Native Barcoding, Rapid Barcoding)

### Custom adapter file (CLI)

Use the `--adapter_file` option to supply custom adapters at runtime. By default, custom adapters are **merged** with the built-in adapter sets — built-in entries take priority on name conflicts. The file uses a pipe-delimited format:

```
# Lines starting with '#' are comments; blank lines are ignored.
# Format: name | start_name,start_seq | end_name,end_seq
#
# Single-ended adapter (no end sequence):
MyAdapter | my_start,ATCGATCGATCG | ,
# Double-ended adapter:
MyPairedAdapter | my_start,ATCGATCG | my_end,CGATCGAT
```

Example invocation (merge with built-ins):

```bash
cporechop -i reads.fastq -o trimmed.fastq --adapter_file custom_adapters.txt
```

To use **only** the custom adapters and skip all built-in sets, add `--no_default_adapters`:

```bash
cporechop -i reads.fastq -o trimmed.fastq --adapter_file custom_adapters.txt --no_default_adapters
```

> `--no_default_adapters` requires `--adapter_file`. When used, the registry starts empty and contains only the adapters from your file.

### Custom adapter sets (API)

When using `libcporechop` programmatically, custom adapter sets can be added to an `AdapterRegistry` in two ways:

**From a file:**

```cpp
#include "porechop/adapter_file.hpp"

porechop::AdapterRegistry registry = porechop::AdapterRegistry::get_builtin();
registry.add_file("/path/to/custom_adapters.txt");
```

**Programmatic construction:**

```cpp
#include "porechop/types.hpp"
#include "porechop/adapter_file.hpp"

auto custom = porechop::AdapterSet::create(
    "My Custom Adapter",
    "custom_start",  "ATCGATCGATCG",
    "custom_end",    "CGATCGATCGAT"
);

porechop::AdapterRegistry registry = porechop::AdapterRegistry::get_builtin();
registry.add(custom);
```

You can also start with an empty registry and populate it entirely with custom adapters:

```cpp
porechop::AdapterRegistry registry;  // empty
registry.add(custom1);
registry.add(custom2);
registry.add_file("more_adapters.txt");
```

When merging (via `add` or `add_file`), existing entries in the registry take priority over new entries with the same name.

### Custom barcode sets

Custom barcode sets follow the same API, but **the adapter set name must start with `"Barcode "`** to be recognized as a barcode by the demultiplexing pipeline. Include `"(forward)"` or `"(reverse)"` for orientation auto-detection. The numeric portion after `"Barcode "` determines the bin label used in output filenames:

| Adapter set name | Bin label |
|---|---|
| `Barcode 97 (forward)` | `BC97` |
| `Barcode 03 (reverse)` | `BC03` |

**Adapter file example:**

```
Barcode 97 (forward) | BC97,AAGAAAGTTGTCGGTGTCTTTGTG | BC97_rev,CACAAAGACACCGACAACTTTCTT
Barcode 98 (forward) | BC98,TTGACTTGACTTGACTTGACTTGA | BC98_rev,TCAAGTCAAGTCAAGTCAAGTCAA
```

**API example:**

```cpp
auto my_barcode = porechop::AdapterSet::create(
    "Barcode 97 (forward)",
    "BC97",     "AAGAAAGTTGTCGGTGTCTTTGTG",
    "BC97_rev", "CACAAAGACACCGACAACTTTCTT"
);
registry.add(my_barcode);
```

> **Important:** Use unique numbers for each custom barcode — two barcode sets with the same number will collide (same bin label). Non-numeric names produce the fallback label `BC00`.

---

## Acknowledgments

cporechop is a derivative work of [Porechop](https://github.com/rrwick/Porechop) by Ryan Wick (rrwick@gmail.com). The original Porechop is licensed under the GNU General Public License v3. This port is a C++20 reimplementation using seqan3 and is likewise distributed under the MIT License.

All credit for the original design, adapter sequences, and algorithms goes to Ryan Wick.

---

## License

MIT License. See [LICENSE](LICENSE) for the full text.
