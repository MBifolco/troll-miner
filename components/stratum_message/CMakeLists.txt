idf_component_register(
SRCS
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