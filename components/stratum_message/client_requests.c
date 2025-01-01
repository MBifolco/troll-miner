#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>

char *build_configure_request() {
    char *configure_req = malloc(REQUEST_BUFFER_SIZE);
    sprintf(configure_req,
            "{\"id\": %d, "
            "\"method\": \"mining.configure\", "
            "\"params\": [[\"version-rolling\"], {\"version-rolling.mask\": \"ffffffff\"}]}\n",
            STRATUM_CONFIGURE_MESSAGE_ID);
    return configure_req;
}

char *build_subscribe_request() {
    char *subscribe_req = malloc(REQUEST_BUFFER_SIZE);
    sprintf(subscribe_req, "{\"id\": %d, \"method\": \"mining.subscribe\", \"params\": [\"trollminer\"]}\n", STRATUM_SUBSCRIBE_MESSAGE_ID);
    return subscribe_req;
}

char *build_authorize_request() {
    char *auth_req = malloc(REQUEST_BUFFER_SIZE);
    sprintf(auth_req, "{\"id\": %d, \"method\": \"mining.authorize\", \"params\": [\"%s\", \"%s\"]}\n", STRATUM_AUTHORIZE_MESSAGE_ID,
            CONFIG_POOL_USER, CONFIG_POOL_PW);
    return auth_req;
}

char *build_suggest_difficulty_request() { return NULL; }

/**
int authenticate_with_pool() {
    // Example: Send an authentication message
    // const char *auth_msg = "{\"id\": 2, \"method\": \"mining.authorize\", \"params\": [\"username\", \"password\"]}\n";
    char auth_msg[1024];
    sprintf(auth_msg, "{\"id\": %d, \"method\": \"mining.authorize\", \"params\": [\"%s\", \"%s\"]}\n", message_id, CONFIG_POOL_USER,
CONFIG_POOL_PW); return pool_send(auth_msg);
}

int suggest_difficulty() {
    // Example: Send a difficulty suggestion message
    // const char *suggest_difficulty_msg = "{\"id\": 3, \"method\": \"mining.suggest_difficulty\", \"params\": [1]}\n";
    char suggest_difficulty_msg[1024];
    sprintf(suggest_difficulty_msg, "{\"id\": %d, \"method\": \"mining.suggest_difficulty\", \"params\": [1000]}\n", message_id);
    return pool_send(suggest_difficulty_msg);
}
*/