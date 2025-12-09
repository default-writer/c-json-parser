#!/usr/bin/bash -e

cwd=$(pwd)

cd "test"

wget -O simdjson.h https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h
wget -O simdjson.cpp https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.cpp

cd "${cwd}"
