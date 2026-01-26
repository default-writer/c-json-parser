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

# cleanup previous coverage files
rm -f ./*/*.gcda ./*/*.gcno ./coverage.info
rm -rf coverage_report

# build with coverage
ninja -f $NINJA_FILE perf-gprof

# run tests to generate coverage data
./test-perf-gprof > /dev/null 2>&1

# generate coverage report
lcov --capture --directory . --output-file coverage.info > /dev/null 2>&1
genhtml coverage.info --output-directory coverage_report

# cleanup build artifacts
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1
rm -f *.gcda *.gcno *.info
rm -f src/*.gcda src/*.gcno
rm -f utils/*.gcda utils/*.gcno
rm -f perf/*.gcda perf/*.gcno
