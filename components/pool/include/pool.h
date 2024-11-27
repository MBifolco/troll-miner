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

// Function to subscribe to pool messages
int subscribe_to_pool();

// Function to authenticate with the pool
int authenticate_with_pool();

// Function to suggest difficulty to the pool
int suggest_difficulty();

#endif // POOL_COMPONENT_H
