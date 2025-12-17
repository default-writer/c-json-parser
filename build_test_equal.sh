#!/usr/bin/bash -e

# cleanup
ninja -f build.linux.test_equal.ninja -t clean > /dev/null 2>&1 || true

# main target
ninja -f build.linux.test_equal.ninja

./test-equal
