#include "../inc/uart.h"
#include <string.h>
#include <fcntl.h>

static Status get_baud_rate(speed_t baud)
{
    switch(baud)
    {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:    
            return B115200;
        default:
            return E_INVAL_BAUD;
    }
}

Status uart_init(uart_handler_t* handler, 
                    const uint8_t* device, 
                    speed_t baud)
{
    Status status = E_NOK;
    if(NULL == handler || NULL == device || baud <= 0)
    {
        return (NULL == handler || NULL == device) ? E_INVAL_POINTER : E_INVAL_BAUD;
    }

    handler->baud_rate = get_baud_rate(baud);

    strncpy((char*)handler->device_path, (const char*)device, sizeof(handler->device_path) - 1);
    handler->device_path[sizeof(handler->device_path) - 1] = '\0';

    handler->fd = open((const char*)handler->device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(handler->fd < 0)
    {
        return E_INVAL_FILE;
    }

    return E_OK;
}

Status uart_transmit(const uart_handler_t* handler, 
                        const uint8_t* data, 
                        const uint8_t length)
{

}

Status uart_receive_timeout(const uart_handler_t* handler, 
                            uint8_t* data, 
                            const uint8_t length,
                            const uint64_t timeout_ms)
{

}

Status uart_receive_continous(const uart_handler_t* handler, 
                                uint8_t* data, 
                                const uint8_t length,
                                const uint64_t duration_ms)
{

}

Status uart_cleanup(const uart_handler_t* handler,)
{

}

