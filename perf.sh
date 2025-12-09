#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# gprof target
ninja -f build.linux.ninja gprof
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

./test-main-gcc

gprof ./test-main-gcc gmon.out > test-main-gcc.txt

[[ -f gmon.out ]] && rm gmon.out
