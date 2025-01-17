idf_component_register(
SRCS
    "asic_request_task.c"
    "asic_result_listener_task.c"
    "fayksic.c"
    "fayksic_result.c"
    "crc.c"
   
    "serial.c"
    "diff_utils.c"

INCLUDE_DIRS 
    "include"
    "../job/include"
    "../../main/"
    "../stats/include"
    "../stratum/include"
    "../utils/include"

REQUIRES 
    "freertos"
    "driver"
    "json"
    "mbedtls"
)