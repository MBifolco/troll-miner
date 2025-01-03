idf_component_register(
SRCS
    "stratum.c"
    "client_requests/mining_authorize.c"
    "client_requests/mining_configure.c"
    "client_requests/mining_subscribe.c"
    "client_requests/mining_suggest_difficulty.c"

    "server_responses/mining_subscribe.c"

    "stratum_message.c"

INCLUDE_DIRS 
    "include"
    "../job/include"
    "../utils/include"
    "../asic/include"
    "../../main/"
    "../pool/include"

REQUIRES
    "json"
    "driver"
)