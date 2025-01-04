idf_component_register(
SRCS
    "pool.c"
    "mock_pool.c"

INCLUDE_DIRS 
    "include"
    "../stratum/include"

REQUIRES
    "json"
)