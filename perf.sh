#!/bin/bash

# Build the performance executable using ninja
echo "Building test-perf-c-json-parser..."
ninja -f build.linux.ninja perf-c-json-parser

if [ ! -f "./test-perf-c-json-parser" ]; then
    echo "Error: test-perf-c-json-parser not found. Build failed."
    exit 1
fi

echo "Running performance tests 100 times..."

# Initialize a temporary file to store times
temp_file=$(mktemp)

num_runs=100

for i in $(seq 1 $num_runs); do
    # Use time command, redirect stderr to capture real time
    # Store only the real time in seconds in the temp file
    # The format of 'time' command output can vary slightly between systems.
    # This specifically targets the 'real' time in seconds.
    # Example: real    0m0.003s -> 0.003
    # Example: real    0m1.234s -> 1.234
    real_time=$( { time ./test-perf-c-json-parser > /dev/null; } 2>&1 | grep real | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print ($1*60)+$2}' )
    
    # Handle potential empty real_time if grep/awk fails
    if [ -z "$real_time" ]; then
        echo "Warning: Could not extract real time for run $i. Skipping this run."
        continue
    fi

    echo "Run $i: ${real_time} seconds"
    echo "${real_time}" >> "$temp_file"
done

# Calculate total and average time from the temp file
# Check if temp_file is empty before awk
if [ -s "$temp_file" ]; then
    total_time=$(awk '{sum+=$1} END {print sum}' "$temp_file")
    average_time=$(awk '{sum+=$1} END {print sum/NR}' "$temp_file")
else
    total_time=0
    average_time=0
fi


echo "-------------------------------------"
echo "Total elapsed time for $num_runs runs: ${total_time} seconds"
echo "Average execution time: ${average_time} seconds"
echo "-------------------------------------"

rm "$temp_file" # Clean up temp file
