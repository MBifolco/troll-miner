#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Build request payload for `mining.configure`
 *
 * String is allocated on the heap - the caller must free the memory themselves.
 *
 * Example: {"id": 1, "method": "mining.configure", "params": [["version-rolling"], {"version-rolling.mask": "ffffffff"}]}
 * Reference: https://reference.cash/mining/stratum-protocol
 */
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