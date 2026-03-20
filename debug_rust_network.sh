#!/bin/bash

# This script is designed to help debug networking issues between the Urukhai Rust server and client.
# It performs the following steps sequentially:
# 1. Navigates to the rust-urukhai directory where the Cargo projects are located.
# 2. Sources the cargo environment script to ensure cargo is in the PATH.
# 3. Compiles the server and client applications using 'cargo build'.
# 4. Creates temporary log files for both the server and client to capture their output.
# 5. Starts the server in the background and logs its process ID (PID).
# 6. Pauses for a second to allow the server to initialize properly.
# 7. Runs the client, which will attempt to connect to the server.
# 8. After the client finishes, it terminates the server process using the stored PID.
# 9. Displays the captured logs from both the server and client for debugging.
# 10. Cleans up the temporary log files.

# Set the base directory relative to the script's location
BASE_DIR=$(dirname "$0")/rust-urukhai
SERVER_LOG="/home/user/.gemini/tmp/84caf131256a79d8b3c6ef046331e3e2490aad13f01e331bd71208638ffe11c1/server.log"
CLIENT_LOG="/home/user/.gemini/tmp/84caf131256a79d8b3c6ef046331e3e2490aad13f01e331bd71208638ffe11c1/client.log"

# Add cargo to the current shell's PATH.
source "$HOME/.cargo/env"

# Kill any lingering server processes
echo "Checking for and stopping any lingering server processes..."
pkill -f urukhai-server || true

# Navigate to the Rust project directory
echo "Navigating to $BASE_DIR..."
cd "$BASE_DIR"

# Build the server and client
echo "Building server and client..."
cargo build

# Run the server in the background
echo "Starting server in the background..."
RUST_LOG=info ./target/debug/urukhai-server > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!
echo "Server started with PID: $SERVER_PID"

# Give the server a moment to start
sleep 1

# Run the client
echo "Running client..."
RUST_LOG=info ./target/debug/urukhai-client --message "The Uruks are coming!" > "$CLIENT_LOG" 2>&1
CLIENT_EXIT_CODE=$?

if [ $CLIENT_EXIT_CODE -ne 0 ]; then
    echo "Client exited with error code: $CLIENT_EXIT_CODE"
fi

# Stop the server
echo "Stopping server..."
kill $SERVER_PID
wait $SERVER_PID || true # wait for the process to exit, ignore error if already stopped

# Output the logs
echo "--- Server Logs ---"
cat "$SERVER_LOG"
echo "--- End of Server Logs ---"

echo ""

echo "--- Client Logs ---"
cat "$CLIENT_LOG"
echo "--- End of Client Logs ---"

# Cleanup
rm -f "$SERVER_LOG" "$CLIENT_LOG"
echo "Cleanup complete."
