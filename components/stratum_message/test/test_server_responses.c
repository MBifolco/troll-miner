#include "cJSON.h"
#include "esp_log.h"
#include "pool.h"
#include "stratum.h"
#include "unity.h"
#include "unity_test_runner.h"

struct pool_session POOL_SESSION;

static cJSON *TEST_JSON;

static void test_setup(void) {
    // Initialize stratum function pointers
    stratum_init();

    // Build the JSON response object
    TEST_JSON = cJSON_CreateObject();
    cJSON_AddNumberToObject(TEST_JSON, "id", 2);
    cJSON_AddNullToObject(TEST_JSON, "error");

    cJSON *result_arr = cJSON_AddArrayToObject(TEST_JSON, "result");
    cJSON *client_metadata = cJSON_AddArrayToObject(result_arr, "result");
    cJSON *method_subscribe = cJSON_CreateString("mining.notify");
    cJSON *subscription_id = cJSON_CreateString("677a0966");
    cJSON_AddItemReferenceToArray(client_metadata, method_subscribe);
    cJSON_AddItemReferenceToArray(client_metadata, subscription_id);

    cJSON *extranonce1 = cJSON_CreateString("19865d67");
    cJSON *extranonce2_len = cJSON_CreateNumber(2);
    cJSON_AddItemReferenceToArray(result_arr, extranonce1);
    cJSON_AddItemReferenceToArray(result_arr, extranonce2_len);

    // Reset the POOL_SESSION members
    POOL_SESSION.extranonce1 = "";
    POOL_SESSION.extranonce2 = NULL;
    POOL_SESSION.extranonce2_len = 8;
}

TEST_CASE("subscribe response handler sets POOL_SESSION vars", "[subscribe_response_handler]") {
    test_setup();

    STRATUM.server_response_handler.subscribe(TEST_JSON);
    TEST_ASSERT_EQUAL_STRING("19865d67", POOL_SESSION.extranonce1);
    TEST_ASSERT_EQUAL(2, POOL_SESSION.extranonce2_len);
    for (int i = 0; i < POOL_SESSION.extranonce2_len; ++i) {
        TEST_ASSERT_EQUAL(0, POOL_SESSION.extranonce2[i]);
    }
}

TEST_CASE("no changes to POOL_SESSION if result key missing from payload", "[subscribe_response_handler]") {
    test_setup();

    cJSON_DeleteItemFromObject(TEST_JSON, "result");
    STRATUM.server_response_handler.subscribe(TEST_JSON);
    TEST_ASSERT_EQUAL_STRING("", POOL_SESSION.extranonce1);
    TEST_ASSERT_EQUAL(8, POOL_SESSION.extranonce2_len);
    TEST_ASSERT_NULL(POOL_SESSION.extranonce2);
}