#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-c-json-parser target
ninja -f build.linux.ninja $1
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 
