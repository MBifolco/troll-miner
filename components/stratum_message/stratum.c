#include "stratum.h"

struct Stratum STRATUM;

void stratum_init() {
    STRATUM.client_request.build_authorize_request = build_authorize_request;
    STRATUM.client_request.build_configure_request = build_configure_request;
    STRATUM.client_request.build_subscribe_request = build_subscribe_request;
    STRATUM.client_request.build_suggest_difficulty_request = build_suggest_difficulty_request;

    // TODO
    // STRATUM.server_response_handler.subscribe = subscribe_response_handler;

    // STRATUM.server_request_handler.notify = notify_request_handler;
    // STRATUM.server_request_handler.set_difficulty = set_difficulty_request_handler;
}