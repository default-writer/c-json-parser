#!/bin/env bash

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

cwd=$(cd "$(dirname $(dirname "${BASH_SOURCE[0]}"))" &> /dev/null && pwd)

BUILD_DIR="${cwd}/build"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

git clone https://github.com/json-c/json-c.git
cd json-c

cmake -DCMAKE_INSTALL_PREFIX=../../libs/json-c -DCMAKE_BUILD_TYPE=release
make all install

cd "${cwd}"

[[ $SHLVL -eq 2 ]] && echo OK

cd "${pwd}"