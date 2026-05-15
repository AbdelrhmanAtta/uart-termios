#!/bin/bash

echo "========================================="
echo "Virtual UART Setup Script for RISC-V Testing"
echo "========================================="

if ! command -v socat &> /dev/null; then
    echo "socat not found. Installing..."
    if command -v apt-get &> /dev/null; then
        sudo apt-get update && sudo apt-get install -y socat
    elif command -v pacman &> /dev/null; then
        sudo pacman -S socat
    else
        echo "Please install socat manually"
        exit 1
    fi
fi

echo "Creating virtual serial ports..."
socat -d -d pty,raw,echo=0,link=/tmp/ttyV0 pty,raw,echo=0,link=/tmp/ttyV1 &

SOCAT_PID=$!
echo "socat PID: $SOCAT_PID"

sleep 2

echo ""
echo "Virtual ports created:"
ls -la /tmp/ttyV*

echo ""
echo "To test with echo server:"
echo "  Terminal 2: cat /tmp/ttyV1"
echo "  Terminal 3: echo 'Hello' > /tmp/ttyV1"
echo ""
echo "Or run the test:"
echo "  make test-run ARGS='/tmp/ttyV0'"
echo ""
echo "For RISC-V cross-compilation:"
echo "  make riscv ARGS='/tmp/ttyV0'"
echo ""
echo "Press Enter to stop socat..."
read

kill $SOCAT_PID
echo "Virtual ports removed."

