#!/usr/bin/env bash
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

target="$1"
if [[ -z "$1" ]]; then
  target="main"
fi

# cleanup
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1 

# main target
ninja -f $NINJA_FILE ${target}
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1

./test-${target}
