#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <termios.h>
#include "utils.h"

#define DEVICE_PATH_LENGTH          64
#define DEFAULT_TIMEOUT_MS          1000
#define DEFAULT_DURATION_MS         5000

typedef struct uart_handler_t
{
    uint8_t device_path[DEVICE_PATH_LENGTH];
    uint32_t fd;
    speed_t baud_rate;
    struct termios old_termios;
}uart_handler_t;


Status uart_init(uart_handler_t* handler, 
                    const uint8_t* device, 
                    speed_t baud);

Status uart_transmit(const uart_handler_t* handler, 
                        const uint8_t* data, 
                        const uint8_t length);

Status uart_receive_timeout(const uart_handler_t* handler, 
                            uint8_t* data, 
                            const uint8_t length,
                            const uint64_t timeout_ms);

Status uart_receive_continous(const uart_handler_t* handler, 
                                uint8_t* data, 
                                const uint8_t length,
                                const uint64_t duration_ms);

Status uart_cleanup(const uart_handler_t* handler,);

#endif /* UART_H */

