#ifndef STRATUM_COMPONENT_H
#define STRATUM_COMPONENT_H

#include "cJSON.h"
#include <stdint.h>

#define REQUEST_BUFFER_SIZE 1024

// Predefined message IDs
#define STRATUM_CONFIGURE_MESSAGE_ID 1
#define STRATUM_SUBSCRIBE_MESSAGE_ID 2
#define STRATUM_AUTHORIZE_MESSAGE_ID 3
#define STRATUM_SUGGEST_DIFFICULTY_MESSAGE_ID 4

void stratum_init();

// Stratum client requests
char *build_configure_request();
char *build_subscribe_request();
char *build_authorize_request();
char *build_suggest_difficulty_request();

// Stratum server responses
void subscribe_response_handler(cJSON *json);

// Stratum server requests
void set_difficulty_request_handler(cJSON *json);
void notify_request_handler(cJSON *json);

// Collection of stratum client request functions
struct client_request {
    char *(*build_configure_request)();          // mining.configure
    char *(*build_subscribe_request)();          // mining.subscribe
    char *(*build_authorize_request)();          // mining.authorize
    char *(*build_suggest_difficulty_request)(); // mining.suggest_diffiuclty
};

// Collection of stratum server response handlers
struct server_response_handler {
    void (*subscribe)(cJSON *json); // mining.subscribe
};

// Collection of stratum server request handlers
struct server_request_handler {
    void (*set_difficulty)(cJSON *json); // mining.set_difficulty
    void (*notify)(cJSON *json);         // mining.notify
};

/**
 * Data structure to store parameters from stratum request/response
 * messages from the pool that inform job creation which generally
 * don't change very frequently, if at all.
 */
struct session {
    char *extranonce1;      // Source: response of mining.subscribe
    uint32_t difficulty;    // Source: request of mining.set_difficulty
    size_t extranonce2_len; // Source: response of mining.subscribe
    uint8_t *extranonce2;   // Global value because we don't need to reset it when notified of a new job
};

struct Stratum {
    struct client_request client_request;
    struct server_response_handler server_response_handler;
    struct server_request_handler server_request_handler;
    struct session session;
};
extern struct Stratum STRATUM;

#endif // STRATUM_COMPONENT_H