idf_component_register(
SRCS
    "job_task.c"

INCLUDE_DIRS 
    "include"
    "../stratum_message/include"
    "../../main/"
    "../utils/include"
REQUIRES
    "json"
    "mbedtls"
)