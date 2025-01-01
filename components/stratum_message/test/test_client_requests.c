#include "cJSON.h"
#include "stratum.h"
#include "unity.h"
#include "unity_test_runner.h"

void setUp(void) { stratum_init(); }

TEST_CASE("mining.configure stratum client request", "[build_configure_request]") {
    char *expected =
        "{\"id\": 1, \"method\": \"mining.configure\", \"params\": [[\"version-rolling\"], {\"version-rolling.mask\": \"ffffffff\"}]}\n";
    char *configure_req = STRATUM.client_request.build_configure_request();
    TEST_ASSERT_EQUAL_STRING(expected, configure_req);
}

TEST_CASE("mining.subscribe stratum client request", "[build_subscribe_request]") {
    char *expected = "{\"id\": 2, \"method\": \"mining.subscribe\", \"params\": [\"trollminer\"]}\n";
    char *configure_req = STRATUM.client_request.build_subscribe_request();
    TEST_ASSERT_EQUAL_STRING(expected, configure_req);
}
