idf_component_register(
SRCS
    "job_task.c"
    "utils.c"

INCLUDE_DIRS 
    "include"
    "../stratum/include"
    "../../main/"
    "../utils/include"
REQUIRES
    "json"
    "mbedtls"
)