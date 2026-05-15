/**
 * @file uart.h
 * @brief UART communication interface for Linux systems
 */

#ifndef UART_H
#define UART_H

#include "utils.h"
#include <sys/types.h>
#include <termios.h>

typedef struct {
    int fd;
    char device_path[64];
    speed_t baud_rate;
    struct termios old_termios;
    int is_initialized;
} uart_handler_t;

uart_status_t uart_init(uart_handler_t* handler, const char* device, int baud);

uart_status_t uart_transmit(const uart_handler_t* handler, 
                           const uint8_t* data, 
                           size_t length);

uart_status_t uart_transmit_string(const uart_handler_t* handler, const char* message);

ssize_t uart_receive_timeout(const uart_handler_t* handler, 
                            uint8_t* data, 
                            size_t length,
                            int timeout_ms);

ssize_t uart_receive_continuous(const uart_handler_t* handler, 
                               uint8_t* data, 
                               size_t length,
                               int duration_ms);

uart_status_t uart_flush(const uart_handler_t* handler);

uart_status_t uart_cleanup(uart_handler_t* handler);

const char* uart_strerror(uart_status_t status);

#endif /* UART_H */

