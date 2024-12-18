idf_component_register(
SRCS
    "stratum_message.c"

INCLUDE_DIRS 
    "include"
    "../job/include"
    "../utils/include"
    "../asic/include"
    "../../main/"

REQUIRES
    "json"
    "driver"
)