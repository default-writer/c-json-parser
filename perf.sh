#!/bin/bash

# Build the performance executable using ninja
# For perf-c-json-parser-no-string-validation, automatically runs
# Profile-Guided Optimization (PGO) workflow:
# 1. Builds instrumented binary with -fprofile-generate
# 2. Runs it 10 times to collect profile data
# 3. Merges profiles with llvm-profdata
# 4. Builds optimized binary with -fprofile-use
# 5. Runs performance tests on optimized binary
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

# PGO workflow for perf-c-json-parser-no-string-validation, perf-c-json-parser-no-string-validation-long,
# perf-c-json-parser, and perf-c-json-parser-long
if [[ "$target" == "perf-c-json-parser-no-string-validation" || "$target" == "perf-c-json-parser-no-string-validation-long" || \
      "$target" == "perf-c-json-parser" || "$target" == "perf-c-json-parser-long" ]]; then
    # Determine if this is the long version
    if [[ "$target" == *"-long" ]]; then
        is_long=true
        # Remove -long from target before adding PGO suffix
        base_target="${target%-long}"
        pgo_generate_suffix="-long-pgo-generate"
        pgo_use_suffix="-long-pgo-use"
    else
        is_long=false
        base_target="$target"
        pgo_generate_suffix="-pgo-generate"
        pgo_use_suffix="-pgo-use"
    fi
    
    # Determine if this is the no-string-validation version
    if [[ "$target" == *"no-string-validation"* ]]; then
        has_validation=false
        if [[ "$is_long" == true ]]; then
            profile_dir="./gprof-no-string-validation-long"
            base_binary_name="test-perf-c-json-parser-no-string-validation"
        else
            profile_dir="./gprof-no-string-validation"
            base_binary_name="test-perf-c-json-parser-no-string-validation"
        fi
    else
        has_validation=true
        if [[ "$is_long" == true ]]; then
            profile_dir="./gprof-validate-long"
            base_binary_name="test-perf-c-json-parser"
        else
            profile_dir="./gprof-validate"
            base_binary_name="test-perf-c-json-parser"
        fi
    fi
    
    echo "Starting PGO (Profile-Guided Optimization) workflow for $target..."
    
    # Clean up any existing profile data
    rm -rf "$profile_dir"
    mkdir -p "$profile_dir"
    
    # Build PGO generate target
    echo "Building PGO instrumented binary..."
    ./build-c-json-parser.sh "${base_target}${pgo_generate_suffix}"
    
    # Run instrumented binary multiple times to collect profile data
    profile_runs=100
    echo "Running instrumented binary $profile_runs times to collect profile data..."
    for i in $(seq 1 $profile_runs); do
        program_output=$( "./${base_binary_name}${pgo_generate_suffix}" 2>&1 )
        if echo "$program_output" | grep -q "PASSED"; then
            echo -n "."
        else
            echo "Failed to run instrumented binary"
            exit 1
        fi
    done
    echo ""
    
    # Merge profile data
    echo "Merging profile data..."
    if command -v llvm-profdata >/dev/null 2>&1; then
        llvm-profdata merge -output="$profile_dir/default.profdata" "$profile_dir"/*.profraw
    else
        # Try to find llvm-profdata in the known location
        if [ -f "/home/user/src/c-simple-allocator/.tools/llvm/llvm-profdata" ]; then
            /home/user/src/c-simple-allocator/.tools/llvm/llvm-profdata merge -output="$profile_dir/default.profdata" "$profile_dir"/*.profraw
        else
            echo "Error: llvm-profdata not found"
            exit 1
        fi
    fi
    
    # Build PGO use target with profile data
    echo "Building PGO-optimized binary..."
    ./build-c-json-parser.sh "${base_target}${pgo_use_suffix}"
    
    # Set the target to the PGO-optimized binary for performance testing
    actual_binary="${base_binary_name}${pgo_use_suffix}"
else
    # Normal build for non-PGO targets
    ./build-c-json-parser.sh ${target}
    actual_binary="test-${target}"
fi

echo "running performance tests 100 times..."

temp_file=$(mktemp)
num_runs=100

for i in $(seq 1 $num_runs); do
    program_output=$( "./${actual_binary}" 2>&1 )
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