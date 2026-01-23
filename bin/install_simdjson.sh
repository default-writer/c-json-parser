#!/usr/bin/env bash
set -e

case "$(uname)" in
    "Darwin")
        echo "Unsupported Operating System: $(uname)"
        exit 1
        ;;
esac

set -e
if [[ "${BASHOPTS}" != *extdebug* ]]; then
    set -e
fi

err_report() {
    cd ${source}
    echo "ERROR: $0:$*"
    exit 8
}

if [[ "${BASHOPTS}" != *extdebug* ]]; then
    trap 'err_report $LINENO' ERR
fi

cwd=$(pwd)

libs_simdjson="libs/simdjson"
mkdir -p $libs_simdjson
cd "$libs_simdjson"

wget -O simdjson.h https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h
wget -O simdjson.cpp https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.cpp

cd "${cwd}"
