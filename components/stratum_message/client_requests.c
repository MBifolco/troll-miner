/**
 * Implementations for stratum.client_requests methods
 *
 * All methods allocate memory on the heap. It's up to the callers
 * to call free on the returned values
 */
#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>

char *build_configure_request() {
    char *configure_req = malloc(REQUEST_BUFFER_SIZE);
    if (configure_req == NULL) {
        return NULL;
    }

    sprintf(configure_req,
            "{\"id\": %d, "
            "\"method\": \"mining.configure\", "
            "\"params\": [[\"version-rolling\"], {\"version-rolling.mask\": \"ffffffff\"}]}\n",
            STRATUM_CONFIGURE_MESSAGE_ID);
    return configure_req;
}

char *build_subscribe_request() {
    char *subscribe_req = malloc(REQUEST_BUFFER_SIZE);
    if (subscribe_req == NULL) {
        return NULL;
    }

    sprintf(subscribe_req, "{\"id\": %d, \"method\": \"mining.subscribe\", \"params\": [\"trollminer\"]}\n", STRATUM_SUBSCRIBE_MESSAGE_ID);
    return subscribe_req;
}

char *build_authorize_request() {
    char *auth_req = malloc(REQUEST_BUFFER_SIZE);
    if (auth_req == NULL) {
        return NULL;
    }

    sprintf(auth_req, "{\"id\": %d, \"method\": \"mining.authorize\", \"params\": [\"%s\", \"%s\"]}\n", STRATUM_AUTHORIZE_MESSAGE_ID,
            CONFIG_POOL_USER, CONFIG_POOL_PW);
    return auth_req;
}

char *build_suggest_difficulty_request() {
    char *suggest_diff_req = malloc(REQUEST_BUFFER_SIZE);
    if (suggest_diff_req == NULL) {
        return NULL;
    }

    sprintf(suggest_diff_req, "{\"id\": %d, \"method\": \"mining.suggest_difficulty\", \"params\": [256]}\n",
            STRATUM_SUGGEST_DIFFICULTY_MESSAGE_ID);
    return suggest_diff_req;
}