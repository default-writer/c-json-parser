#!/bin/bash

# This script installs the necessary requirements for building Rust projects.

# Exit script if any command fails
set -e

# Check if Rust is already installed by looking for the 'cargo' command.
if command -v cargo &> /dev/null
then
    echo "Rust is already installed."
else
    echo "Rust is not installed. Installing Rust..."
    
    # Install Rust using rustup. The '-y' flag automates the installation.
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    
    # Add cargo to the current shell's PATH.
    source "$HOME/.cargo/env"
    
    echo "Rust has been installed successfully."
fi

# Verify the installation by checking the versions of rustc and cargo.
echo "Verifying installation..."
rustc --version
cargo --version

echo "All requirements for building Rust sources are installed and ready to use."
