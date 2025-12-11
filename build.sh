#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# main target
ninja -f build.linux.ninja
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# mem target
ninja -f build.linux.ninja mem
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf target
ninja -f build.linux.ninja perf
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-long target
ninja -f build.linux.ninja perf-long
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-alloc target
ninja -f build.linux.ninja perf-alloc
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-alloc-long target
ninja -f build.linux.ninja perf-alloc-long
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# test-json-parse target
ninja -f build.linux.ninja test-json-parse
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# test-simple-array target
ninja -f build.linux.ninja test-array
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# test-single-object-array target
ninja -f build.linux.ninja test-object
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

