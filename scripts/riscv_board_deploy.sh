#!/bin/bash

echo "========================================="
echo "RISC-V Board Deployment Script"
echo "========================================="

if [ $# -lt 1 ]; then
    echo "Usage: $0 <board_ip> [username] [device]"
    echo "Example: $0 192.168.1.100 root /dev/ttyS0"
    exit 1
fi

BOARD_IP=$1
USERNAME=${2:-root}
DEVICE=${3:-/dev/ttyS0}
BAUD=${4:-115200}
MODE=${5:-test}

echo "Deploying to RISC-V board at $BOARD_IP"

make riscv

if [ ! -f .build/uart_app_riscv ]; then
    echo "RISC-V binary not found. Compilation failed."
    exit 1
fi

echo "Copying binary to board..."
scp .build/uart_app_riscv $USERNAME@$BOARD_IP:/tmp/

echo "Running UART test on board..."
ssh $USERNAME@$BOARD_IP "chmod +x /tmp/uart_app_riscv && sudo /tmp/uart_app_riscv $DEVICE $BAUD $MODE"

echo "Done!"

