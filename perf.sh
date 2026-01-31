#!/bin/bash

# Build the performance executable using ninja
echo "building test-perf-c-json-parser..."

target="$1"
if [[ -z "$1" ]]; then
  target="perf-c-json-parser"
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

./build-c-json-parser.sh ${target}

echo "running performance tests 100 times..."

temp_file=$(mktemp)
num_runs=100

for i in $(seq 1 $num_runs); do
    program_output=$( "./test-${target}" 2>&1 )
    raw_time=$(echo "$program_output" | grep "execution time:" | awk '{print $NF}' | sed 's/.*00:\([0-9]*\)\.\([0-9]*\)/\1\.\2/')
    real_time=$(echo "$raw_time" | sed 's/^00\./0./')
    if [ -z "$real_time" ]; then
        continue
    fi
    printf "run %4d: %.3f seconds\n" "$i" "$real_time"
    echo "${real_time}" >> "$temp_file"
done

if [ -s "$temp_file" ]; then
    total_time=$(awk '{sum+=$1} END {print sum}' "$temp_file")
    average_time=$(awk '{sum+=$1} END {print sum/NR}' "$temp_file")
    min_time=$(awk 'BEGIN {min = -1} {if ($1 < min || min == -1) min = $1} END {print min}' "$temp_file")
else
    total_time=0
    average_time=0
    min_time=0
fi

echo "--------------------------------------------"
printf "%-26s %9.3f seconds\n" "total elapsed time:" "$total_time"
printf "%-26s %9.3f seconds\n" "average execution time:" "$average_time"
printf "%-26s %9.3f seconds\n" "minimum execution time:" "$min_time"
echo "--------------------------------------------"

rm "$temp_file" # Clean up temp file
