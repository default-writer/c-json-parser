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
  target="gprof-coverage"
fi

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

./cleanup.sh

# cleanup previous coverage files
rm -f ./*/*.gcda ./*/*.gcno ./coverage.info ./*/*.gcov ./*/*.out test-${target}
rm -rf coverage_report

# build with coverage
ninja -f $NINJA_FILE ${target}

# run tests to generate coverage data
./test-${target} > /dev/null 2>&1

# generate coverage report
lcov --capture --directory . --rc geninfo_unexecuted_blocks=1 --output-file coverage.info #> /dev/null 2>&1
genhtml coverage.info --output-directory coverage_report

# cleanup build artifacts
ninja -f $NINJA_FILE -t clean > /dev/null 2>&1

rm -f *.gcda *.gcno *.info *.gcov *.out test-${target}
