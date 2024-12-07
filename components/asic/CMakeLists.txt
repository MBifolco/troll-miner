idf_component_register(
SRCS
    "asic.c"
    "common.c"
    "fayksic.c"
    "serial.c"
    "crc.c"

INCLUDE_DIRS 
    "include"
    "../job/include"
    "../../main/"
    "../utils/include"

REQUIRES 
    "freertos"
    "driver"
)