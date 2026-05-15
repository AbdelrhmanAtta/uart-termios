# RISC-V UART Communication Library

## Brief Description

A C library for UART serial communication on Linux/RISC-V systems using termios API. Supports non-blocking I/O with timeouts, multiple baud rates, and comprehensive error handling.

## API Functions

| Function | Description |
|----------|-------------|
| uart_init() | Initialize UART device |
| uart_transmit() | Send data |
| uart_transmit_string() | Send string |
| uart_receive_timeout() | Receive with timeout |
| uart_receive_continuous() | Receive for duration |
| uart_cleanup() | Close and restore settings |
| uart_strerror() | Get error description |

## Build

```
# Native build
make compile

# Build test suite
make test

# RISC-V cross-compile
make riscv

# Clean
make clean
```

## Run

```
# Main application
sudo .build/uart_app /dev/ttyUSB0 115200 test

# Test suite
sudo .build/uart_test /dev/ttyUSB0 115200 5

# Virtual ports (no hardware)
socat -d -d pty,link=/tmp/ttyV0 pty,link=/tmp/ttyV1
.build/uart_test /tmp/ttyV0
```

## Modes

| Mode | Command |
|------|---------|
| Test | .build/uart_app /dev/ttyUSB0 115200 test |
| Receive | .build/uart_app /dev/ttyUSB0 115200 receive |
| Send | .build/uart_app /dev/ttyUSB0 115200 send |
| Loopback | .build/uart_app /dev/ttyUSB0 115200 loopback |

## Test Numbers

| # | Test |
|---|------|
| 1 | Basic transmit/receive |
| 2 | Continuous receive |
| 3 | Loopback |
| 4 | Error handling |
| 5 | All tests |

## Example

```
#include "uart.h"

uart_handler_t uart;
uart_status_t status;

// Initialize
status = uart_init(&uart, "/dev/ttyUSB0", 115200);

// Send
uart_transmit_string(&uart, "Hello\r\n");

// Receive
uint8_t buf[256];
ssize_t len = uart_receive_timeout(&uart, buf, sizeof(buf), 2000);

// Cleanup
uart_cleanup(&uart);
```

## RISC-V Board

```
# Cross-compile
make riscv

# Copy to board
scp .build/uart_app_riscv root@192.168.1.100:/tmp/

# Run on board
ssh root@192.168.1.100 '/tmp/uart_app_riscv /dev/ttyS0 115200 test'
```

## QEMU Simulation

```
# Terminal 1
qemu-system-riscv64 -M virt -nographic -serial pty -kernel .build/uart_app_riscv

# Terminal 2 (use the /dev/pts/X shown above)
screen /dev/pts/2 115200
```

## Error Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| -1 | General error |
| -2 | Invalid pointer |
| -3 | Invalid baud rate |
| -4 | Invalid device |
| -5 | Permission denied |
| -6 | Timeout |
| -7 | I/O error |
| -8 | Not initialized |

## Quick Test with Virtual Ports

```
# Terminal 1 - Create virtual ports
socat -d -d pty,link=/tmp/ttyV0 pty,link=/tmp/ttyV1

# Terminal 2 - Run echo server
while true; do read -r line < /tmp/ttyV1 && echo "$line" > /tmp/ttyV1; done

# Terminal 3 - Run test
make test
.build/uart_test /tmp/ttyV0
```

## Troubleshooting

```
# Permission denied
sudo chmod 666 /dev/ttyUSB0
# OR
sudo usermod -a -G dialout $USER

# Device not found
ls -la /dev/tty*
dmesg | grep tty

# Check baud rate
stty -F /dev/ttyUSB0 -a
```

## Files

```
inc/
  ├── utils.h
  └── uart.h
src/
  ├── uart.c
  └── main.c
test/
  └── uart_test.c
Makefile
```

This documentation covers all aspects of building, testing, and using the UART library on RISC-V systems.

