idf_component_register(
SRCS
    "test_client_requests.c"
    "test_server_requests.c"
    "test_server_responses.c"

    # External deps that linker will need at compile time
    "../../asic/fayksic.c"

INCLUDE_DIRS 
    "../include"
    "../../asic/include"
    "../../job/include"
    "../../pool/include"
    "../../utils/include"

REQUIRES
    "unity"
    "json"
    "driver"
)