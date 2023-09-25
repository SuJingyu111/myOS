#ifndef STATUS_H
#define STATUS_H

typedef enum __MYOS_STATUS {
    MYOS_HEALTHY = 0,
    EIO,
    EINVARG,
    ENOMEM
} MYOS_STATUS;

#endif