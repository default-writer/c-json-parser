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

if [[ "${BASHOPTS}" != *extdebug* ]]; then
    set -e
fi

err_report() {
    cd ${cwd}
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

# perf-json-c target
ninja -f $NINJA_FILE perf-json-c 
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1 

# perf-json-c-long target
ninja -f $NINJA_FILE perf-json-c-long
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1
