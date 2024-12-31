#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>

char *build_configure_request() {
    char *configure_msg = malloc(REQUEST_BUFFER_SIZE);
    sprintf(configure_msg,
            "{\"id\": %d, "
            "\"method\": \"mining.configure\", "
            "\"params\": [[\"version-rolling\"], {\"version-rolling.mask\": \"ffffffff\"}]}\n",
            STRATUM_CONFIGURE_MESSAGE_ID);

    return configure_msg;
}

char *build_subscribe_request() { return NULL; }
char *build_authorize_request() { return NULL; }
char *build_suggest_difficulty_request() { return NULL; }
/*
int configure_version_rolling() {
    // Example: Send a version rolling message
    // const char *version_rolling_msg = "{\"id\": %d, \"method\": \"mining.configure\", \"params\": [{\"version-rolling.mask\": \"0x0000000f\",
    // \"version-rolling.min-difficulty\": 1, \"version-rolling.max-difficulty\": 1, \"version-rolling.threshold\": 1}]}\n";
    char version_rolling_msg[1024];
    sprintf(version_rolling_msg,
            "{\"id\": %d, "
            "\"method\": \"mining.configure\", "
            "\"params\": [[\"version-rolling\"], {\"version-rolling.mask\": \"ffffffff\"}]}\n",
            message_id);

    return pool_send(version_rolling_msg);
}

int subscribe_to_pool() {
    // Example: Send a subscription message
    // const char *subscribe_msg = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": []}\n";
    char subscribe_msg[1024];
    sprintf(subscribe_msg, "{\"id\": %d, \"method\": \"mining.subscribe\", \"params\": [\"trollminer\"]}\n", MINING_SUBSCRIBE_MSG_ID);
    return pool_send(subscribe_msg);
}

int authenticate_with_pool() {
    // Example: Send an authentication message
    // const char *auth_msg = "{\"id\": 2, \"method\": \"mining.authorize\", \"params\": [\"username\", \"password\"]}\n";
    char auth_msg[1024];
    sprintf(auth_msg, "{\"id\": %d, \"method\": \"mining.authorize\", \"params\": [\"%s\", \"%s\"]}\n", message_id, CONFIG_POOL_USER, CONFIG_POOL_PW);
    return pool_send(auth_msg);
}

int suggest_difficulty() {
    // Example: Send a difficulty suggestion message
    // const char *suggest_difficulty_msg = "{\"id\": 3, \"method\": \"mining.suggest_difficulty\", \"params\": [1]}\n";
    char suggest_difficulty_msg[1024];
    sprintf(suggest_difficulty_msg, "{\"id\": %d, \"method\": \"mining.suggest_difficulty\", \"params\": [1000]}\n", message_id);
    return pool_send(suggest_difficulty_msg);
}*/