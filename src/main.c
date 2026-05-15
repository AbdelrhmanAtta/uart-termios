/**
 * @file main.c
 * @brief Main program for UART communication
 */

#include "../inc/uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

static uart_handler_t g_uart;
static volatile int g_running = 1;

void signal_handler(int sig)
{
    (void)sig;
    printf("\n\nInterrupt received. Cleaning up...\n");
    g_running = 0;
}

void print_hex(const uint8_t* data, size_t len)
{
    size_t i;
    printf("Hex: ");
    for (i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\nASCII: ");
    for (i = 0; i < len; i++) {
        printf("%c", isprint(data[i]) ? data[i] : '.');
    }
    printf("\n");
}

void print_usage(const char* prog_name)
{
    printf("Usage: %s <device> [baud_rate] [mode]\n", prog_name);
    printf("\nParameters:\n");
    printf("  device      - UART device (e.g., /dev/ttyUSB0, /dev/ttyS0)\n");
    printf("  baud_rate   - Baud rate: 9600, 19200, 38400, 57600, 115200 (default)\n");
    printf("  mode        - Operation mode:\n");
    printf("                test     - Send test message and receive response\n");
    printf("                receive  - Continuous receive mode\n");
    printf("                send     - Interactive send mode\n");
    printf("                loopback - Loopback test\n");
    printf("\nExamples:\n");
    printf("  %s /dev/ttyUSB0\n", prog_name);
    printf("  %s /dev/ttyS0 115200 test\n", prog_name);
    printf("  %s /dev/ttyUSB0 9600 receive\n", prog_name);
}

void mode_test(void)
{
    uart_status_t status;
    ssize_t rx_len;
    uint8_t tx_buffer[] = "Hello from UART test program!\r\n";
    uint8_t rx_buffer[UART_BUFFER_SIZE];
    
    printf("\n=== TEST MODE ===\n");
    
    status = uart_transmit(&g_uart, tx_buffer, sizeof(tx_buffer) - 1);
    if (status == UART_OK) {
        printf("[OK] Test message sent\n");
    } else {
        printf("[FAIL] Failed to send: %s\n", uart_strerror(status));
        return;
    }
    
    printf("Waiting for response (timeout: %d ms)...\n", UART_TIMEOUT_MS);
    rx_len = uart_receive_timeout(&g_uart, rx_buffer, sizeof(rx_buffer) - 1, UART_TIMEOUT_MS);
    
    if (rx_len > 0) {
        rx_buffer[rx_len] = '\0';
        printf("\n[RX] Received %zd bytes:\n", rx_len);
        printf("Text: %s\n", rx_buffer);
        print_hex(rx_buffer, rx_len);
    } else if (rx_len == UART_ERROR_TIMEOUT) {
        printf("[INFO] No data received (timeout)\n");
    } else {
        printf("[ERROR] Receive failed: %s\n", uart_strerror((uart_status_t)rx_len));
    }
}

void mode_receive(void)
{
    ssize_t rx_len;
    uint8_t rx_buffer[UART_BUFFER_SIZE];
    
    printf("\n=== RECEIVE MODE ===\n");
    printf("Listening for %d seconds...\n", UART_CONTINUOUS_MS / 1000);
    printf("Press Ctrl+C to stop\n\n");
    
    rx_len = uart_receive_continuous(&g_uart, rx_buffer, sizeof(rx_buffer) - 1, UART_CONTINUOUS_MS);
    
    if (rx_len > 0) {
        rx_buffer[rx_len] = '\0';
        printf("\n[RX] Received %zd bytes:\n", rx_len);
        printf("Text: %s\n", rx_buffer);
        print_hex(rx_buffer, rx_len);
    } else if (rx_len == UART_ERROR_TIMEOUT) {
        printf("\n[INFO] No data received\n");
    } else {
        printf("\n[ERROR] Receive failed: %s\n", uart_strerror((uart_status_t)rx_len));
    }
}

void mode_send(void)
{
    char input[UART_BUFFER_SIZE];
    uart_status_t status;
    
    printf("\n=== SEND MODE ===\n");
    printf("Enter messages to send (type 'quit' to exit):\n\n");
    
    while (g_running) {
        printf("> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        
        if (strlen(input) == 0) {
            continue;
        }
        
        status = uart_transmit_string(&g_uart, input);
        if (status == UART_OK) {
            printf("[OK] Sent: %s\n", input);
        } else {
            printf("[FAIL] Send failed: %s\n", uart_strerror(status));
        }
        
        usleep(100000);
    }
}

void mode_loopback(void)
{
    uart_status_t status;
    ssize_t rx_len;
    uint8_t tx_buffer[] = "LOOPBACK_TEST_MESSAGE_12345\r\n";
    uint8_t rx_buffer[UART_BUFFER_SIZE];
    
    printf("\n=== LOOPBACK TEST ===\n");
    printf("Connect TX to RX for this test\n");
    
    status = uart_transmit(&g_uart, tx_buffer, sizeof(tx_buffer) - 1);
    if (status != UART_OK) {
        printf("[FAIL] Transmit failed: %s\n", uart_strerror(status));
        return;
    }
    printf("[OK] Sent test message (%zu bytes)\n", sizeof(tx_buffer) - 1);
    
    usleep(200000);
    
    rx_len = uart_receive_timeout(&g_uart, rx_buffer, sizeof(rx_buffer) - 1, 2000);
    
    if (rx_len > 0) {
        rx_buffer[rx_len] = '\0';
        
        if (rx_len == (ssize_t)(sizeof(tx_buffer) - 1) &&
            memcmp(tx_buffer, rx_buffer, rx_len) == 0) {
            printf("[PASS] Loopback test passed! Data matches.\n");
        } else {
            printf("[FAIL] Loopback test failed! Data mismatch.\n");
            printf("Sent (%zu bytes): %s\n", sizeof(tx_buffer) - 1, tx_buffer);
            printf("Received (%zd bytes): %s\n", rx_len, rx_buffer);
        }
        print_hex(rx_buffer, rx_len);
    } else if (rx_len == UART_ERROR_TIMEOUT) {
        printf("[FAIL] No data received. Check TX-RX connection.\n");
    } else {
        printf("[FAIL] Receive error: %s\n", uart_strerror((uart_status_t)rx_len));
    }
}

int main(int argc, char* argv[])
{
    uart_status_t status;
    const char* device;
    int baud = UART_DEFAULT_BAUD;
    const char* mode = "test";
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    device = argv[1];
    
    if (argc >= 3) {
        baud = atoi(argv[2]);
        if (baud <= 0) {
            printf("Error: Invalid baud rate\n");
            return 1;
        }
    }
    
    if (argc >= 4) {
        mode = argv[3];
    }
    
    printf("\n========================================\n");
    printf("UART Communication Program\n");
    printf("========================================\n");
    printf("Device: %s\n", device);
    printf("Baud rate: %d\n", baud);
    printf("Mode: %s\n", mode);
    printf("========================================\n\n");
    
    status = uart_init(&g_uart, device, baud);
    if (status != UART_OK) {
        printf("Failed to initialize UART: %s\n", uart_strerror(status));
        printf("\nTroubleshooting:\n");
        printf("  - Check device path: %s\n", device);
        printf("  - Run with sudo: sudo %s ...\n", argv[0]);
        printf("  - Add user to dialout group: sudo usermod -a -G dialout $USER\n");
        return 1;
    }
    
    printf("UART initialized successfully!\n");
    printf("  Device: %s\n", g_uart.device_path);
    printf("  FD: %d\n", g_uart.fd);
    
    if (strcmp(mode, "test") == 0) {
        mode_test();
    } else if (strcmp(mode, "receive") == 0) {
        mode_receive();
    } else if (strcmp(mode, "send") == 0) {
        mode_send();
    } else if (strcmp(mode, "loopback") == 0) {
        mode_loopback();
    } else {
        printf("Error: Unknown mode '%s'\n", mode);
        print_usage(argv[0]);
    }
    
    uart_cleanup(&g_uart);
    printf("\nProgram terminated.\n");
    
    return 0;
}

