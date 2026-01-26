#!/bin/bash
set -e

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

# cleanup
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1 

# perf-simdjson
ninja -f $NINJA_FILE perf-simdjson
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1 

# perf-simdjson-long
ninja -f $NINJA_FILE perf-simdjson-long
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1

