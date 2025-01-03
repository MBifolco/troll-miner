#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Build request payload for `mining.suggest_difficulty`
 *
 * String is allocated on the heap - the caller must free the memory themselves.
 *
 * Example: {"id": 3, "method": "mining.suggest_difficulty", "params": [256]}
 * Reference: https://reference.cash/mining/stratum-protocol
 */
char *build_suggest_difficulty_request() {
    char *suggest_diff_req = malloc(REQUEST_BUFFER_SIZE);
    if (suggest_diff_req == NULL) {
        return NULL;
    }

    sprintf(suggest_diff_req, "{\"id\": %d, \"method\": \"mining.suggest_difficulty\", \"params\": [256]}\n",
            STRATUM_SUGGEST_DIFFICULTY_MESSAGE_ID);
    return suggest_diff_req;
}