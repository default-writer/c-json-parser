#!/usr/bin/bash -e

target="$1"
if [[ -z "$1" ]] then
  target="perf-c-json-parser"
fi

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# perf-c-json-parser target
ninja -f build.linux.ninja $target
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

./test-${target}