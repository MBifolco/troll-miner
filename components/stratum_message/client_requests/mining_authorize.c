#include "sdkconfig.h"
#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Build request payload for `mining.authorize`
 *
 * String is allocated on the heap - the caller must free the memory themselves.
 *
 * Example: {"id": 3, "method": "mining.authorize", "params": ["username", "password"]}
 * Reference: https://reference.cash/mining/stratum-protocol
 */
char *build_authorize_request() {
    char *auth_req = malloc(REQUEST_BUFFER_SIZE);
    if (auth_req == NULL) {
        return NULL;
    }

    sprintf(auth_req, "{\"id\": %d, \"method\": \"mining.authorize\", \"params\": [\"%s\", \"%s\"]}\n", STRATUM_AUTHORIZE_MESSAGE_ID,
            CONFIG_POOL_USER, CONFIG_POOL_PW);
    return auth_req;
}
