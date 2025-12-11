# C JSON parser

- lightning-fast implementation of a JSON parser
- c89 compatible
- easy to read, learn and implement
- performance improvement rate of x4/x4 for speed/memory compared to [json-c](https://github.com/json-c/json-c)
- just a little bit slower (x0.5) than [simdjson](https://github.com/simdjson/simdjson)

## badges

[![CodeQL](https://github.com/default-writer/c-json-parser/actions/workflows/codeql.yml/badge.svg?branch=main)](https://github.com/default-writer/c-json-parser/actions/workflows/codeql.yml)

## Speed comparison

| Metric                                  |            simdjson |    c-json-parser(*) |       c-json-parser |              json-c |
| :---------------------------------------| ------------------: | ------------------: | ------------------: | ------------------: |
| execution time (100K run)               |        00:00:00.395 |        00:00:00.881 |        00:00:01.182 |        00:00:04.093 |
| execution time (1M runs)                |                   - |        00:00:08.808 |        00:00:11.838 |        00:00:42.702 |
| allocation calls (100K runs)            |                   - |                   0 |          20,000,000 |          52,900,000 |
| allocation calls (1M runs)              |                   - |                   0 |         200,000,000 |         529,000,000 |
| total heap usage (100K runs)            |                   - |                   0 |         806,400,000 |       4,179,600,000 |
| total heap usage (1M runs)              |                   - |                   0 |       8,064,000,000 |      41,796,000,000 |

(*) - alloc-free version (fixed buffer size)

## docs

- [data structures](docs/data-structures.md)
- [documentation](docs)

## screenshots

![logo_c_json_parser](images/image.png)

## tools

Ninja + Clang

## prerequisites

```bash
sudo apt install -y git cmake clang clang gcc g++ lld
```

## simdjson / json-c

```bash
./bin/install_simdjson.sh
./bin/install_json_c.sh
```

## build

```bash
./build.sh
```

```bash
./build-json-c.sh
```

or

```bash
ninja -f build.linux.ninja && ./test-main
```

## test

```bash
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja && ninja -f build.linux.ninja -t clean > /dev/null 2>&1
```

## profiling / performance tests (json-c)

```bash
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja perf && ninja -f build.linux.ninja -t clean > /dev/null 2>&1
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja perf-json-c && ninja -f build.linux.ninja -t clean > /dev/null 2>&1
```

## python

```bash
python3 test/perf_test.py
```
