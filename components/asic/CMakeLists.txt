idf_component_register(
SRCS
    "asic_request_task.c"
    "fayksic.c"
    "crc.c"
    "common.c"
    "serial.c"

INCLUDE_DIRS 
    "include"
    "../job/include"
    "../../main/"
    "../stratum_message/include"
    "../utils/include"

REQUIRES 
    "freertos"
    "driver"
    "json"
)