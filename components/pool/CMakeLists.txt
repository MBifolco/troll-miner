idf_component_register(
SRCS
    "pool.c"
    "mock_pool.c"

INCLUDE_DIRS 
    "include"
    "../stratum_message/include"

REQUIRES
    "json"
)