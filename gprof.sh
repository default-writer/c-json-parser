#!/bin/bash -e

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

# gprof target
ninja -f $NINJA_FILE gprof
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1

./test-gprof

gprof ./test-gprof gmon.out > test-gprof.txt

[[ -f gmon.out ]] && rm gmon.out
