#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# gprof target
ninja -f build.linux.ninja gprof
ninja -f build.linux.ninja -t clean > /dev/null 2>&1

./test-gprof

gprof ./test-gprof gmon.out > test-gprof.txt

[[ -f gmon.out ]] && rm gmon.out
