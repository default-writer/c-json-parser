#!/bin/bash

NINJA_FILE="build.linux.ninja"
case "$(uname)" in
    "Darwin")
        NINJA_FILE="build.osx.ninja"
        ;;
    "Linux")
        NINJA_FILE="build.linux.ninja"
        ;;
    *)
        echo "Unsupported Operating System: $(uname)"
        exit 1
        ;;
esac

target="$1"
if [[ -z "$1" ]]; then
  target="perf-c-json-parser"
fi

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
cd ${cwd}

# cleanup
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1 

# perf-c-json-parser target
ninja -f $NINJA_FILE $target
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1 
