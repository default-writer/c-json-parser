#!/usr/bin/env bash


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
rm ./test-* > /dev/null 2>&1
rm -f *.gcda *.gcno *.ultra *.o gmon.out perf.data test-gprof.txt perf-gprof.txt > /dev/null 2>&1 || true
