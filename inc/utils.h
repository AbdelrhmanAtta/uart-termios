/**
 * @file utils.h
 * @brief Utility definitions and error codes for UART communication
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

#define UART_BUFFER_SIZE    256
#define UART_TIMEOUT_MS     2000
#define UART_CONTINUOUS_MS  5000
#define UART_DEFAULT_BAUD   115200
#define UART_DATA_BITS      8
#define UART_PARITY_NONE    0
#define UART_STOP_BITS_1    1

typedef enum {
    UART_OK = 0,
    UART_ERROR = -1,
    UART_ERROR_INVAL_POINTER,
    UART_ERROR_INVAL_BAUD,
    UART_ERROR_INVAL_DEVICE,
    UART_ERROR_PERMISSION,
    UART_ERROR_TIMEOUT,
    UART_ERROR_IO,
    UART_ERROR_NOT_INITIALIZED
} uart_status_t;

#endif /* UTILS_H */

