#!/usr/bin/bash -e

cwd=$(pwd)

cd "test"

wget -O simdjson.h https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h
wget -O simdjson.cpp https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.cpp

cd ".."
# perf-simdjson
ninja -f build.linux.ninja perf-simdjson
ninja -f build.linux.ninja -t clean > /dev/null 2>&1 

cd "${cwd}"
