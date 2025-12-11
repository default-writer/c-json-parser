#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# main target
ninja -f build.linux.ninja
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# mem target
ninja -f build.linux.ninja mem
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# array target
ninja -f build.linux.ninja array
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# array target
ninja -f build.linux.ninja object
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

# json-parse target
ninja -f build.linux.ninja json-parse
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 
