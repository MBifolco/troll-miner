#include "../../build/config/sdkconfig.h" // Manually reference build sdkconfig.h - Unity + ESP point to some test default
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
    free(configure_req);
}

TEST_CASE("mining.subscribe stratum client request", "[build_subscribe_request]") {
    char *expected = "{\"id\": 2, \"method\": \"mining.subscribe\", \"params\": [\"trollminer\"]}\n";
    char *subscribe_req = STRATUM.client_request.build_subscribe_request();
    TEST_ASSERT_EQUAL_STRING(expected, subscribe_req);
    free(subscribe_req);
}

TEST_CASE("mining.authorize stratum client request", "[build_authorize_request]") {
    char expected[1024];
    sprintf(expected, "{\"id\": 3, \"method\": \"mining.authorize\", \"params\": [\"%s\", \"%s\"]}\n", CONFIG_POOL_USER, CONFIG_POOL_PW);
    char *authorize_req = STRATUM.client_request.build_authorize_request();
    TEST_ASSERT_EQUAL_STRING(expected, authorize_req);
    free(authorize_req);
}

TEST_CASE("mining.suggest_difficulty stratum client request", "[build_suggest_difficulty_request]") {
    char *expected = "{\"id\": 4, \"method\": \"mining.suggest_difficulty\", \"params\": [256]}\n";
    char *suggest_diff_req = STRATUM.client_request.build_suggest_difficulty_request();
    TEST_ASSERT_EQUAL_STRING(expected, suggest_diff_req);
    free(suggest_diff_req);
}
