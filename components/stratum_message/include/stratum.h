#ifndef STRATUM_COMPONENT_H
#define STRATUM_COMPONENT_H

#include "cJSON.h"

#define REQUEST_BUFFER_SIZE 1024

// Predefined message IDs
#define STRATUM_CONFIGURE_MESSAGE_ID 1

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

struct Stratum {
    struct client_request client_request;
    struct server_response_handler server_response_handler;
    struct server_request_handler server_request_handler;
};
extern struct Stratum STRATUM;

#endif // STRATUM_COMPONENT_H