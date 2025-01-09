#include "cJSON.h"
#include "stratum.h"
#include "unity.h"
#include "unity_test_runner.h"

static cJSON *TEST_JSON;

static void test_setup(void) {
    // Initialize stratum function pointers
    stratum_init();

    /**
     * Build the server's response JSON object
     *
     * {"result":[["mining.notify","677a0966"],"19865d67",2],"id":2,"error":null}",
     */
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
}

static void test_teardown() { cJSON_Delete(TEST_JSON); }

TEST_CASE("subscribe response handler sets STRATUM.session vars", "[subscribe_response_handler]") {
    test_setup();

    STRATUM.server_response_handler.subscribe(TEST_JSON);
    TEST_ASSERT_EQUAL_STRING("19865d67", STRATUM.session.extranonce1);
    TEST_ASSERT_EQUAL(2, STRATUM.session.extranonce2_len);
    for (int i = 0; i < STRATUM.session.extranonce2_len; ++i) {
        TEST_ASSERT_EQUAL(0, STRATUM.session.extranonce2[i]);
    }

    test_teardown();
}

TEST_CASE("no changes to STRATUM.session if result key missing from payload", "[subscribe_response_handler]") {
    test_setup();

    cJSON_DeleteItemFromObject(TEST_JSON, "result");
    STRATUM.server_response_handler.subscribe(TEST_JSON);
    TEST_ASSERT_EQUAL_STRING("", STRATUM.session.extranonce1);
    TEST_ASSERT_EQUAL(8, STRATUM.session.extranonce2_len);
    TEST_ASSERT_NULL(STRATUM.session.extranonce2);

    test_teardown();
}

TEST_CASE("mining.subscribe response with missing values from `result` does not modify STRATUM.session", "[subscribe_response_handler]") {
    test_setup();

    cJSON *result_arr = cJSON_GetObjectItem(TEST_JSON, "result");
    cJSON_DeleteItemFromArray(result_arr, 2);

    STRATUM.server_response_handler.subscribe(TEST_JSON);
    TEST_ASSERT_EQUAL_STRING("", STRATUM.session.extranonce1);
    TEST_ASSERT_EQUAL(8, STRATUM.session.extranonce2_len);
    TEST_ASSERT_NULL(STRATUM.session.extranonce2);

    test_teardown();
}

TEST_CASE("mining.subscribe response with an error does not end up modifying STRATUM.session", "[subscribe_response_handler]") {
    test_setup();

    cJSON_DeleteItemFromObject(TEST_JSON, "error");
    cJSON *err_arr = cJSON_AddArrayToObject(TEST_JSON, "error");
    cJSON *err_msg = cJSON_CreateString("oops!");
    cJSON_AddItemToArray(err_arr, err_msg);

    STRATUM.server_response_handler.subscribe(TEST_JSON);
    TEST_ASSERT_EQUAL_STRING("", STRATUM.session.extranonce1);
    TEST_ASSERT_EQUAL(8, STRATUM.session.extranonce2_len);
    TEST_ASSERT_NULL(STRATUM.session.extranonce2);

    test_teardown();
}