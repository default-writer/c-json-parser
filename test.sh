#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# main target
ninja -f build.linux.ninja
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

./test-main
