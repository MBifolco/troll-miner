idf_component_register(
SRCS
    "test_client_requests.c"
    "test_server_responses.c"
    "server_requests/test_mining_set_difficulty.c"
    "server_requests/test_mining_notify.c"

    # External deps that linker will need at compile time (because we're not mocking)
    "../../asic/fayksic.c"

INCLUDE_DIRS 
    "../include"
    "../../asic/include"
    "../../job/include"
    "../../pool/include"
    "../../utils/include"
    "../../../main" # Bleh - have to do this because of queue_handles.h
    # TODO: move queue_handles.h out of main/

REQUIRES
    "unity"
    "json"
    "driver"
)