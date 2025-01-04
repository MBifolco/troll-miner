#include "stratum.h"

struct stratum STRATUM;

void stratum_init() {
    /**
     * Set intelligent defaults for the session
     */
    STRATUM.session.difficulty = 256;
    STRATUM.session.extranonce1 = "";
    STRATUM.session.extranonce2 = NULL;
    STRATUM.session.extranonce2_len = 8;

    STRATUM.client_request.build_authorize_request = build_authorize_request;
    STRATUM.client_request.build_configure_request = build_configure_request;
    STRATUM.client_request.build_subscribe_request = build_subscribe_request;
    STRATUM.client_request.build_suggest_difficulty_request = build_suggest_difficulty_request;

    STRATUM.server_request_handler.notify = notify_request_handler;
    STRATUM.server_request_handler.set_difficulty = set_difficulty_request_handler;

    STRATUM.server_response_handler.subscribe = subscribe_response_handler;
}