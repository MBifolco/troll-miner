idf_component_register(
SRCS
    "stratum.c"
    "client_requests.c"
    "stratum_message.c"
    "responses/mining_subscribe.c"

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