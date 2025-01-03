#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Build request payload for `mining.subscribe`
 *
 * String is allocated on the heap - the caller must free the memory themselves.
 *
 * Example: {"id": 2, "method": "mining.subscribe", "params": ["trollminer"]}
 * Reference: https://reference.cash/mining/stratum-protocol
 */
char *build_subscribe_request() {
    char *subscribe_req = malloc(REQUEST_BUFFER_SIZE);
    if (subscribe_req == NULL) {
        return NULL;
    }

    sprintf(subscribe_req, "{\"id\": %d, \"method\": \"mining.subscribe\", \"params\": [\"trollminer\"]}\n", STRATUM_SUBSCRIBE_MESSAGE_ID);
    return subscribe_req;
}
