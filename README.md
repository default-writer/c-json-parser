# C JSON Parser

- High-performance JSON parser in C.
- 4x faster and 4x more memory efficient than `json-c`.

## Benchmarks

| Metric | c-json-parser | json-c |
| :--- | ---: | ---: |
| Execution Time (1M runs) | 11.8s | 42.7s |
| Allocation Calls (1M runs) | 200,000,000 | 529,000,000 |
| Heap Usage (1M runs) | 8.06 GB | 41.79 GB |

## Documentation

- [Data Structures](docs/data-structures.md)
- [Further Documentation](docs)

## Build

- **Toolchain**: Ninja, Clang
- **Dependencies**:
  - Run `./bin/install_json_c.sh` to install `json-c` for comparison benchmarks.
- **Build**:
  - Run `./build.sh` or `ninja -f build.linux.ninja`.
- **Test**:
  - `ninja -f build.linux.ninja test`
- **Performance**:
  - `ninja -f build.linux.ninja perf`
  - `ninja -f build.linux.ninja perf-json-c`
  - `python3 test/perf_test.py`
