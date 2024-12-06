idf_component_register(
SRCS
    "job.c"

INCLUDE_DIRS 
    "include"
    "../stratum_message/include"
    "../../main/"
REQUIRES
    "json"
)