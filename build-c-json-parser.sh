#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-c-json-parser target
ninja -f build.linux.ninja perf-c-json-parser
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-c-json-parser-long target
ninja -f build.linux.ninja perf-c-json-parser-long
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 
