/**
 * @file uart.c
 * @brief UART communication implementation
 */

#include "../inc/uart.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>

static speed_t uart_baud_to_speed(int baud)
{
    switch(baud)
    {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        default:     return 0;
    }
}

static uart_status_t uart_configure(int fd, speed_t baud_rate)
{
    struct termios tio;
    
    if (tcgetattr(fd, &tio) != 0) {
        return UART_ERROR_IO;
    }
    
    tio.c_cflag = baud_rate | CS8 | CREAD | CLOCAL;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;
    
    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        return UART_ERROR_IO;
    }
    
    return UART_OK;
}

uart_status_t uart_init(uart_handler_t* handler, const char* device, int baud)
{
    speed_t baud_speed;
    uart_status_t status;
    
    if (!handler || !device) {
        return UART_ERROR_INVAL_POINTER;
    }
    
    baud_speed = uart_baud_to_speed(baud);
    if (baud_speed == 0) {
        return UART_ERROR_INVAL_BAUD;
    }
    
    memset(handler, 0, sizeof(uart_handler_t));
    strncpy(handler->device_path, device, sizeof(handler->device_path) - 1);
    handler->baud_rate = baud_speed;
    
    handler->fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (handler->fd < 0) {
        if (errno == EACCES || errno == EPERM) {
            return UART_ERROR_PERMISSION;
        }
        return UART_ERROR_INVAL_DEVICE;
    }
    
    if (tcgetattr(handler->fd, &handler->old_termios) != 0) {
        close(handler->fd);
        return UART_ERROR_IO;
    }
    
    status = uart_configure(handler->fd, handler->baud_rate);
    if (status != UART_OK) {
        close(handler->fd);
        return status;
    }
    
    tcflush(handler->fd, TCIFLUSH);
    handler->is_initialized = 1;
    
    return UART_OK;
}

uart_status_t uart_transmit(const uart_handler_t* handler, 
                           const uint8_t* data, 
                           size_t length)
{
    ssize_t bytes_written;
    size_t total_written = 0;
    
    if (!handler || !handler->is_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }
    
    if (!data || length == 0) {
        return UART_ERROR_INVAL_POINTER;
    }
    
    while (total_written < length) {
        bytes_written = write(handler->fd, data + total_written, length - total_written);
        if (bytes_written < 0) {
            if (errno == EINTR) continue;
            return UART_ERROR_IO;
        }
        total_written += bytes_written;
    }
    
    return UART_OK;
}

uart_status_t uart_transmit_string(const uart_handler_t* handler, const char* message)
{
    if (!message) {
        return UART_ERROR_INVAL_POINTER;
    }
    return uart_transmit(handler, (const uint8_t*)message, strlen(message));
}

ssize_t uart_receive_timeout(const uart_handler_t* handler, 
                            uint8_t* data, 
                            size_t length,
                            int timeout_ms)
{
    fd_set read_fds;
    struct timeval timeout;
    int select_result;
    ssize_t bytes_read;
    
    if (!handler || !handler->is_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }
    
    if (!data || length == 0) {
        return UART_ERROR_INVAL_POINTER;
    }
    
    FD_ZERO(&read_fds);
    FD_SET(handler->fd, &read_fds);
    
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    
    select_result = select(handler->fd + 1, &read_fds, NULL, NULL, &timeout);
    
    if (select_result < 0) {
        return UART_ERROR_IO;
    } else if (select_result == 0) {
        return UART_ERROR_TIMEOUT;
    }
    
    bytes_read = read(handler->fd, data, length);
    
    if (bytes_read < 0) {
        return UART_ERROR_IO;
    }
    
    return bytes_read;
}

ssize_t uart_receive_continuous(const uart_handler_t* handler, 
                               uint8_t* data, 
                               size_t length,
                               int duration_ms)
{
    fd_set read_fds;
    struct timeval timeout;
    struct timeval start_time, current_time;
    ssize_t bytes_read;
    int elapsed_ms;
    
    if (!handler || !handler->is_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }
    
    if (!data || length == 0) {
        return UART_ERROR_INVAL_POINTER;
    }
    
    gettimeofday(&start_time, NULL);
    
    while (1) {
        gettimeofday(&current_time, NULL);
        elapsed_ms = (current_time.tv_sec - start_time.tv_sec) * 1000;
        elapsed_ms += (current_time.tv_usec - start_time.tv_usec) / 1000;
        
        if (elapsed_ms >= duration_ms) {
            break;
        }
        
        FD_ZERO(&read_fds);
        FD_SET(handler->fd, &read_fds);
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        
        if (select(handler->fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            bytes_read = read(handler->fd, data, length);
            if (bytes_read > 0) {
                return bytes_read;
            }
        }
    }
    
    return UART_ERROR_TIMEOUT;
}

uart_status_t uart_flush(const uart_handler_t* handler)
{
    if (!handler || !handler->is_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }
    
    tcflush(handler->fd, TCIFLUSH);
    return UART_OK;
}

uart_status_t uart_cleanup(uart_handler_t* handler)
{
    if (!handler) {
        return UART_ERROR_INVAL_POINTER;
    }
    
    if (handler->is_initialized && handler->fd >= 0) {
        tcsetattr(handler->fd, TCSANOW, &handler->old_termios);
        close(handler->fd);
        handler->is_initialized = 0;
    }
    
    return UART_OK;
}

const char* uart_strerror(uart_status_t status)
{
    switch (status) {
        case UART_OK:                       return "Success";
        case UART_ERROR:                    return "General error";
        case UART_ERROR_INVAL_POINTER:      return "Invalid pointer";
        case UART_ERROR_INVAL_BAUD:         return "Invalid baud rate";
        case UART_ERROR_INVAL_DEVICE:       return "Invalid device";
        case UART_ERROR_PERMISSION:         return "Permission denied";
        case UART_ERROR_TIMEOUT:            return "Operation timeout";
        case UART_ERROR_IO:                 return "I/O error";
        case UART_ERROR_NOT_INITIALIZED:    return "UART not initialized";
        default:                            return "Unknown error";
    }
}

