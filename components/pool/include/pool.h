#ifndef POOL_COMPONENT_H
#define POOL_COMPONENT_H

#include <stddef.h>
#include <stdint.h>

// TODO: Add the rest of the message IDs so we can use them both in pool & stratum_message
#define MINING_SUBSCRIBE_MSG_ID 2

/**
 * Data structure to store parameters from stratum request/response
 * messages from the pool that inform job creation which generally
 * don't change very frequently, if at all.
 */
struct pool_session {
    char *extranonce1;      // Source: response of mining.subscribe
    uint32_t difficulty;    // Source: request of mining.set_difficulty
    size_t extranonce2_len; // Source: response of mining.subscribe
    uint8_t *extranonce2;   // Global value because we don't need to reset it when notified of a new job
};
extern struct pool_session POOL_SESSION;

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

const char *getaddrinfo_error(int err);

#endif // POOL_COMPONENT_H
