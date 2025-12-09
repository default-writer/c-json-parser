#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-simdjson
ninja -f build.linux.ninja perf-simdjson
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-simdjson-long
ninja -f build.linux.ninja perf-simdjson-long
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 
