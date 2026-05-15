#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

enum Status
{
    E_OK    =   0,
    E_NOK   =   1,
    E_INVAL_BAUD,
    E_INVAL_POINTER,
    E_INVAL_FILE,
    E_MEM_ALLOC_FAIL
};

#endif /* UTILS_H */

