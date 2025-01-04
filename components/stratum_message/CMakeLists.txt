idf_component_register(
SRCS
    "stratum.c"
    "client_requests/mining_authorize.c"
    "client_requests/mining_configure.c"
    "client_requests/mining_subscribe.c"
    "client_requests/mining_suggest_difficulty.c"
    "server_requests/mining_notify.c"
    "server_requests/mining_set_difficulty.c"
    "server_responses/mining_subscribe.c"

    "stratum_message.c"

INCLUDE_DIRS 
    "include"
    "../../main/"
    "../asic/include"
    "../job/include"
    "../pool/include"
    "../utils/include"

REQUIRES
    "json"
    "driver"
)