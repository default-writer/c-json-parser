#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# main target
ninja -f build.linux.ninja
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# memory target
ninja -f build.linux.ninja memory
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

# perf-json-c target
ninja -f build.linux.ninja perf-json-c 
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-json-c-long target
ninja -f build.linux.ninja perf-json-c-long
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 
