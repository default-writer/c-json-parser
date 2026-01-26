#!/usr/bin/env bash
set -e

./build-c-json-parser.sh perf-c-json-parser

perf record -g --call-graph dwarf ./test-perf-c-json-parser > /dev/null
perf report
