#!/bin/bash

# Script to decode stack trace addresses from PIOS kernel
# Usage: ./decode_trace.sh <address1> <address2> ...
# Or pipe stack trace output: your_program | ./decode_trace.sh

ARMGNU="./arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf"

# Auto-detect which ELF file to use, or allow override via ELF_FILE env var
if [ -z "$ELF_FILE" ]; then
    # Find the most recently modified ELF file
    NEWEST_ELF=$(ls -t build/kernel8*.elf 2>/dev/null | head -1)

    if [ -n "$NEWEST_ELF" ]; then
        ELF_FILE="$NEWEST_ELF"
        echo "Auto-detected ELF: $ELF_FILE" >&2
    else
        echo "Error: No ELF file found in build/. Run 'make' first." >&2
        exit 1
    fi
else
    echo "Using specified ELF: $ELF_FILE" >&2
fi

if [ ! -f "$ELF_FILE" ]; then
    echo "Error: $ELF_FILE not found." >&2
    exit 1
fi

decode_address() {
    local addr=$1
    # Remove any brackets, whitespace, etc.
    addr=$(echo "$addr" | sed 's/[^0-9a-fxA-FX]//g')

    if [ -n "$addr" ]; then
        echo "Address: $addr"
        ${ARMGNU}-addr2line -e "$ELF_FILE" -f -i "$addr" | sed 's/^/  /'
        echo ""
    fi
}

# If arguments provided, decode them
if [ $# -gt 0 ]; then
    for addr in "$@"; do
        decode_address "$addr"
    done
else
    # Read from stdin and look for hex addresses
    while IFS= read -r line; do
        echo "$line"
        # Extract hex addresses from lines like "  [0] 0xffff0000000821ac"
        if echo "$line" | grep -qE '\[.*\].*0x[0-9a-fA-F]+'; then
            addr=$(echo "$line" | grep -oE '0x[0-9a-fA-F]+')
            if [ -n "$addr" ]; then
                ${ARMGNU}-addr2line -e "$ELF_FILE" -f -i "$addr" | sed 's/^/    -> /'
            fi
        fi
    done
fi
