/*
  By- Krtin Jain
 */
#include "kernel_types.h"
#include "mutex.h"

typedef struct {
    mutex_t *mutex;
    uint32_t udp_port;
} udp_rx_args_t;

kernel_pid_t udp_rx_init(void *args);