# simple JSON parser in C

A simple JSON parser in C

## tools

Ninja + Clang

## prerequisites (json-c)

```bash
mkdir build
cd build
git clone https://github.com/json-c/json-c.git
cd json-c
cmake -DCMAKE_INSTALL_PREFIX=../../libs/
make all test install
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
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 && ninja -f build.linux.ninja perf
```
