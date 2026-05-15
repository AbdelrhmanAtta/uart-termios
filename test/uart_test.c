#include "../inc/uart.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static uart_handler_t g_uart;
static volatile int g_running = 1;

void signal_handler(int sig)
{
    (void)sig;
    printf("\n\nInterrupt received. Cleaning up...\n");
    g_running = 0;
}

void print_status(ssize_t status, const char* operation)
{
    if (status == UART_OK) {
        printf("[PASS] %s successful\n", operation);
    } else if (status > 0) {
        printf("[INFO] %s received %zd bytes\n", operation, status);
    } else if (status == UART_ERROR_TIMEOUT) {
        printf("[INFO] %s: No data received (timeout)\n", operation);
    } else {
        printf("[INFO] %s: %s\n", operation, uart_strerror((uart_status_t)status));
    }
}

void print_usage(const char* prog_name)
{
    printf("Usage: %s <device> [baud_rate] [test_number]\n", prog_name);
    printf("\nParameters:\n");
    printf("  device      - UART device (e.g., /dev/ttyUSB0, /dev/ttyS0)\n");
    printf("  baud_rate   - Baud rate: 9600, 19200, 38400, 57600, 115200 (default)\n");
    printf("  test_number - Test to run:\n");
    printf("                1 - Basic transmit/receive\n");
    printf("                2 - Continuous receive\n");
    printf("                3 - Loopback test\n");
    printf("                4 - Error handling\n");
    printf("                5 - All tests (default)\n");
}

void test_transmit_receive(void)
{
    uart_status_t status;
    ssize_t rx_len;
    uint8_t tx_buffer[] = "Hello from UART test!\r\n";
    uint8_t rx_buffer[256];
    
    printf("\n=== TEST 1: Basic Transmit and Receive ===\n");
    
    status = uart_transmit(&g_uart, tx_buffer, strlen((char*)tx_buffer));
    print_status(status, "Transmit");
    
    sleep(1);
    
    rx_len = uart_receive_timeout(&g_uart, rx_buffer, sizeof(rx_buffer) - 1, 2000);
    if (rx_len > 0) {
        rx_buffer[rx_len] = '\0';
        printf("[INFO] Received: %s", (char*)rx_buffer);
        print_status(rx_len, "Receive");
    } else {
        print_status(rx_len, "Receive");
    }
}

void test_continuous_receive(void)
{
    ssize_t rx_len;
    uint8_t rx_buffer[256];
    
    printf("\n=== TEST 2: Continuous Receive for 5 seconds ===\n");
    printf("Send data to the UART device now (5 second window)...\n");
    
    rx_len = uart_receive_continuous(&g_uart, rx_buffer, sizeof(rx_buffer) - 1, 5000);
    
    if (rx_len > 0) {
        rx_buffer[rx_len] = '\0';
        printf("[INFO] Received: %s\n", (char*)rx_buffer);
        print_status(rx_len, "Continuous receive");
    } else {
        print_status(rx_len, "Continuous receive");
    }
}

void test_loopback(void)
{
    uart_status_t status;
    ssize_t rx_len;
    uint8_t tx_buffer[] = "LOOPBACK_TEST_12345\r\n";
    uint8_t rx_buffer[256];
    
    printf("\n=== TEST 3: Loopback Test ===\n");
    printf("Note: Connect TX to RX for this test\n");
    
    status = uart_transmit(&g_uart, tx_buffer, strlen((char*)tx_buffer));
    print_status(status, "Transmit");
    
    usleep(100000);
    
    rx_len = uart_receive_timeout(&g_uart, rx_buffer, sizeof(rx_buffer) - 1, 1000);
    
    if (rx_len > 0) {
        rx_buffer[rx_len] = '\0';
        if (strcmp((char*)tx_buffer, (char*)rx_buffer) == 0) {
            printf("[PASS] Loopback test passed! Data matches.\n");
        } else {
            printf("[FAIL] Loopback test failed! Data mismatch.\n");
            printf("  Sent: %s", (char*)tx_buffer);
            printf("  Received: %s", (char*)rx_buffer);
        }
        print_status(rx_len, "Loopback receive");
    } else {
        print_status(rx_len, "Loopback receive");
        printf("[FAIL] No data received in loopback test\n");
    }
}

void test_error_conditions(void)
{
    uart_handler_t invalid_handler;
    uart_status_t status;
    ssize_t rx_len;
    uint8_t buffer[10];
    
    printf("\n=== TEST 4: Error Handling ===\n");
    
    status = uart_init(NULL, "/dev/ttyUSB0", 115200);
    print_status(status, "Init with NULL handler");
    
    status = uart_init(&invalid_handler, NULL, 115200);
    print_status(status, "Init with NULL device");
    
    status = uart_init(&invalid_handler, "/dev/invalid_device", 115200);
    print_status(status, "Init with invalid device");
    
    status = uart_transmit(NULL, buffer, 10);
    print_status(status, "Transmit with NULL handler");
    
    rx_len = uart_receive_timeout(NULL, buffer, 10, 100);
    print_status(rx_len, "Receive with NULL handler");
    
    rx_len = uart_receive_continuous(NULL, buffer, 10, 100);
    print_status(rx_len, "Continuous receive with NULL handler");
    
    status = uart_cleanup(NULL);
    print_status(status, "Cleanup with NULL handler");
}

int main(int argc, char* argv[])
{
    uart_status_t status;
    const char* device;
    int baud = 115200;
    int test_number = 5;
    
    signal(SIGINT, signal_handler);
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    device = argv[1];
    
    if (argc >= 3) {
        baud = atoi(argv[2]);
        if (baud != 9600 && baud != 19200 && baud != 38400 && 
            baud != 57600 && baud != 115200) {
            printf("Error: Invalid baud rate %d. Using default 115200\n", baud);
            baud = 115200;
        }
    }
    
    if (argc >= 4) {
        test_number = atoi(argv[3]);
        if (test_number < 1 || test_number > 5) {
            printf("Error: Invalid test number. Running all tests.\n");
            test_number = 5;
        }
    }
    
    printf("\n========================================\n");
    printf("UART Test Suite\n");
    printf("========================================\n");
    printf("Device: %s\n", device);
    printf("Baud rate: %d\n", baud);
    printf("Test: %d\n", test_number);
    printf("========================================\n");
    
    status = uart_init(&g_uart, device, baud);
    if (status != UART_OK) {
        printf("Failed to initialize UART! Error: %s\n", uart_strerror(status));
        return 1;
    }
    printf("\nUART initialized successfully!\n");
    
    switch (test_number) {
        case 1:
            test_transmit_receive();
            break;
        case 2:
            test_continuous_receive();
            break;
        case 3:
            test_loopback();
            break;
        case 4:
            test_error_conditions();
            break;
        case 5:
            test_transmit_receive();
            test_continuous_receive();
            test_loopback();
            test_error_conditions();
            break;
        default:
            printf("Invalid test number!\n");
            break;
    }
    
    status = uart_cleanup(&g_uart);
    print_status(status, "Cleanup");
    
    printf("\n========================================\n");
    printf("Test completed!\n");
    printf("========================================\n");
    
    return 0;
}

