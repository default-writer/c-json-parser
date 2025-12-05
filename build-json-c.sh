#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-json-c target
ninja -f build.linux.ninja perf-json-c 
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-json-c-long target
ninja -f build.linux.ninja perf-json-c-long
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 
