# simple JSON parser in C

A simple JSON parser in C

- lightning-fast implementation for a reasonable trade-offs, performance x4.009 comared to [json-c](https://github.com/json-c/json-c)

## badges

[![CodeQL](https://github.com/default-writer/c-json-parser/actions/workflows/codeql.yml/badge.svg?branch=main)](https://github.com/default-writer/c-json-parser/actions/workflows/codeql.yml)

## docs

- [documentation](https://github.com/default-writer/c-json-parser/tree/main/docs)

## screenshots

![logo_c_json_parser](images/image-1.png)
![logo_c_json_parser](images/image-2.png)

## tools

Ninja + Clang

## prerequisites (json-c)

```bash
mkdir build
cd build
git clone https://github.com/json-c/json-c.git
cd json-c
cmake -DCMAKE_INSTALL_PREFIX=../../libs/ -DCMAKE_BUILD_TYPE=release
make all install
```

or just run (linux-based)

```bash
./bin/install_json_c.sh
```

## building

```bash
ninja -f build.linux.ninja && ./test-main
```

## testing

```bash
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja && valgrind -s ./test-main
```

## profiling / performance tests (json-c)

```bash
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja o2
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja o3
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja perf
```

## python

```bash
python3 test/perf_test.py
```
