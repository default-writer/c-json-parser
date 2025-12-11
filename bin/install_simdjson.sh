#!/usr/bin/bash -e

cwd=$(pwd)

libs_simdjson="libs/simdjson"
mkdir -p $libs_simdjson
cd "$libs_simdjson"

wget -O simdjson.h https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h
wget -O simdjson.cpp https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.cpp

cd "${cwd}"
