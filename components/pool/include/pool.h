#ifndef POOL_COMPONENT_H
#define POOL_COMPONENT_H

#include <stdint.h>
#include <stddef.h>

// Initialize the pool component
void pool_init();

// Send a message to the pool
int pool_send(const char *message);

// Persistent task for managing the pool connection
void pool_task();

// Function to get version rolling info from pool
int configure_version_rolling();

#endif // POOL_COMPONENT_H
