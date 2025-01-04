#include "cJSON.h"
#include "fayksic.h"
#include "stratum.h"
#include "unity.h"
#include "unity_test_runner.h"

static cJSON *TEST_JSON;

static void test_setup(void) {
    // Initialize stratum function pointers
    stratum_init();

    /**
     * Build the server's request JSON object
     *
     * {"id": null, "method": "mining.set_difficulty", "params": [1024]},
     */
    TEST_JSON = cJSON_CreateObject();
    cJSON_AddNullToObject(TEST_JSON, "id");
    cJSON_AddStringToObject(TEST_JSON, "method", "mining.set_difficulty");

    cJSON *params_arr = cJSON_AddArrayToObject(TEST_JSON, "params");
    cJSON *diff = cJSON_CreateNumber(1024);
    cJSON_AddItemToArray(params_arr, diff);
}

static void test_teardown() { cJSON_Delete(TEST_JSON); }

TEST_CASE("mining.set_difficulty stratum server request stores diff on STRATUM.session", "[set_difficulty_request_handler]") {
    test_setup();

    STRATUM.server_request_handler.set_difficulty(TEST_JSON);
    TEST_ASSERT_EQUAL(1024, STRATUM.session.difficulty);

    test_teardown();
}

TEST_CASE("mining.set_difficulty with bad `params` does not modify STRATUM.session", "[set_difficulty_request_handler]") {
    test_setup();

    // Empty params array
    cJSON *params_arr = cJSON_GetObjectItem(TEST_JSON, "params");
    cJSON_DeleteItemFromArray(params_arr, 0);

    STRATUM.server_request_handler.set_difficulty(TEST_JSON);
    TEST_ASSERT_EQUAL(256, STRATUM.session.difficulty);

    // params key is missing
    cJSON_DeleteItemFromObject(TEST_JSON, "params");
    STRATUM.server_request_handler.set_difficulty(TEST_JSON);
    TEST_ASSERT_EQUAL(256, STRATUM.session.difficulty);

    test_teardown();
}