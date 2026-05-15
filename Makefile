CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinc
RISCV_CC = riscv64-linux-gnu-gcc
RISCV_CFLAGS = -Wall -Wextra -O2 -Iinc -static
LDFLAGS = -lm
TARGET = .build/uart_app
TEST_TARGET = .build/uart_test
RISCV_TARGET = .build/uart_app_riscv
RISCV_TEST_TARGET = .build/uart_test_riscv
ARGS = /dev/ttyUSB0

SRCS = src/uart.c src/main.c
TEST_SRCS = src/uart.c test/uart_test.c

.PHONY: all compile run clean test test1 test2 test3 test4 help qemu riscv riscv-test

all: compile run

compile:
	@mkdir -p .build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)
	@echo "Compilation successful! Binary: $(TARGET)"

riscv:
	@mkdir -p .build
	@if ! command -v $(RISCV_CC) &> /dev/null; then \
		echo "RISC-V cross-compiler not found. Installing..."; \
		if command -v apt-get &> /dev/null; then \
			sudo apt-get install -y gcc-riscv64-linux-gnu; \
		elif command -v pacman &> /dev/null; then \
			sudo pacman -S riscv64-linux-gnu-gcc; \
		else \
			echo "Please install riscv64-linux-gnu-gcc manually"; \
			exit 1; \
		fi \
	fi
	$(RISCV_CC) $(RISCV_CFLAGS) $(SRCS) -o $(RISCV_TARGET)
	@echo "RISC-V compilation successful! Binary: $(RISCV_TARGET)"

test:
	@mkdir -p .build
	$(CC) $(CFLAGS) $(TEST_SRCS) -o $(TEST_TARGET)
	@echo "Test compilation successful! Binary: $(TEST_TARGET)"

riscv-test:
	@mkdir -p .build
	@if ! command -v $(RISCV_CC) &> /dev/null; then \
		echo "RISC-V cross-compiler not found. Please install it first."; \
		exit 1; \
	fi
	$(RISCV_CC) $(RISCV_CFLAGS) $(TEST_SRCS) -o $(RISCV_TEST_TARGET)
	@echo "RISC-V test compilation successful! Binary: $(RISCV_TEST_TARGET)"

run: compile
	@echo "========================================="
	@echo "Running UART Application"
	@echo "========================================="
	./$(TARGET) $(ARGS)

run-sudo: compile
	sudo ./$(TARGET) $(ARGS)

run-riscv: riscv
	@echo "========================================="
	@echo "Running RISC-V UART Application (QEMU)"
	@echo "========================================="
	@echo "Make sure QEMU is running with: ./scripts/qemu_uart.sh"
	@echo "Then run: qemu-system-riscv64 -M virt -nographic -serial pty -kernel $(RISCV_TARGET)"
	@echo ""
	@echo "Or run on actual hardware: scp $(RISCV_TARGET) user@riscv-board:/tmp/"
	@echo "  ssh user@riscv-board 'sudo /tmp/uart_app_riscv /dev/ttyS0'"

test-run: test
	@echo "========================================="
	@echo "Running UART Test Suite"
	@echo "========================================="
	./$(TEST_TARGET) $(ARGS)

test1: test
	./$(TEST_TARGET) $(ARGS) 115200 1

test2: test
	./$(TEST_TARGET) $(ARGS) 115200 2

test3: test
	./$(TEST_TARGET) $(ARGS) 115200 3

test4: test
	./$(TEST_TARGET) $(ARGS) 115200 4

clean:
	rm -rf .build

qemu:
	@echo "========================================="
	@echo "RISC-V QEMU Simulation Setup"
	@echo "========================================="
	@echo "Terminal 1 (QEMU):"
	@echo "  qemu-system-riscv64 -M virt -nographic -serial pty -bios none -kernel /dev/null"
	@echo ""
	@echo "Terminal 2 (Native test):"
	@echo "  make test-run ARGS='/dev/pts/X'"
	@echo ""
	@echo "Terminal 2 (RISC-V test):"
	@echo "  qemu-system-riscv64 -M virt -nographic -serial pty -kernel $(RISCV_TARGET)"
	@echo ""
	@echo "For RISC-V Linux environment:"
	@echo "  qemu-system-riscv64 -M virt -nographic -serial mon:stdio -kernel vmlinux -append 'console=ttyS0'"
	@echo "  Then run: $(RISCV_TARGET) /dev/ttyS0"

help:
	@echo "Available targets:"
	@echo "  make all           - Build and run (default)"
	@echo "  make compile       - Build main application"
	@echo "  make riscv         - Cross-compile for RISC-V"
	@echo "  make test          - Build test suite"
	@echo "  make riscv-test    - Cross-compile test for RISC-V"
	@echo "  make run           - Run main application"
	@echo "  make run-sudo      - Run with sudo"
	@echo "  make run-riscv     - Show RISC-V run instructions"
	@echo "  make test-run      - Run test suite"
	@echo "  make test1-4       - Run specific tests"
	@echo "  make clean         - Remove build directory"
	@echo "  make qemu          - Show QEMU instructions"
	@echo "  make help          - Show this help"
	@echo ""
	@echo "RISC-V Board Deployment:"
	@echo "  scp $(RISCV_TARGET) user@riscv-board:/tmp/"
	@echo "  ssh user@riscv-board 'sudo /tmp/uart_app_riscv /dev/ttyS0'"
	@echo ""
	@echo "Examples:"
	@echo "  make run ARGS='/dev/ttyS0'"
	@echo "  make test-run ARGS='/dev/ttyUSB0 9600'"
	@echo "  make riscv ARGS='/dev/ttyS0'"
	@echo "  sudo .build/uart_app /dev/ttyUSB0 115200 test"

