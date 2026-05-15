#!/bin/bash

echo "========================================="
echo "RISC-V QEMU UART Simulation Script"
echo "========================================="

if ! command -v qemu-system-riscv64 &> /dev/null; then
    echo "QEMU for RISC-V not found. Installing..."
    if command -v apt-get &> /dev/null; then
        sudo apt-get update && sudo apt-get install -y qemu-system-misc
    elif command -v pacman &> /dev/null; then
        sudo pacman -S qemu-system-riscv
    else
        echo "Please install qemu-system-riscv64 manually"
        exit 1
    fi
fi

echo "Starting QEMU for RISC-V with virtual serial port..."
echo "The virtual serial port will be created as /dev/pts/X"
echo "Look for 'char device redirected to /dev/pts/X' in output"
echo ""
echo "In another terminal, run:"
echo "  make run ARGS='/dev/pts/X'"
echo ""

qemu-system-riscv64 -M virt -nographic -serial pty -bios none -kernel /dev/null

