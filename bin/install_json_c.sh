#!/usr/bin/env bash

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

cwd=$(cd "$(dirname $(dirname "${BASH_SOURCE[0]}"))" &> /dev/null && pwd)
BUILD_DIR="${cwd}/build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

git clone https://github.com/json-c/json-c.git
cd json-c

cmake -DCMAKE_INSTALL_PREFIX=../../libs/ -DCMAKE_BUILD_TYPE=release
make all install

cd "${cwd}"

[[ $SHLVL -eq 2 ]] && echo OK

cd "${pwd}"