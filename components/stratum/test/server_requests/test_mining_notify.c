#include "cJSON.h"
#include "fayksic.h"
#include "queue_handles.h"
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
     * {
     *      "id": null,
     *      "method": "mining.notify",
     *      "params": [
     *          "674320f700005d59",
     *          "50120119172a610421a6c3011dd330d9df07b63616c2cc1f1cd0020000000000",
     *          "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b",
     *          "ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000",
     *          [
     *              "c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff",
     *              "49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e"
     *          ],
     *          "01000000",
     *          "4c86041b",
     *          "37221b4d",
     *          true
     *      ]
     * }
     */
    TEST_JSON = cJSON_CreateObject();
    cJSON_AddNullToObject(TEST_JSON, "id");
    cJSON_AddStringToObject(TEST_JSON, "method", "mining.notify");

    cJSON *branch1 = cJSON_CreateString("c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff");
    cJSON *branch2 = cJSON_CreateString("49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e");
    cJSON *merkle_branches_arr = cJSON_CreateArray();
    cJSON_AddItemToArray(merkle_branches_arr, branch1);
    cJSON_AddItemToArray(merkle_branches_arr, branch2);

    cJSON *job_id = cJSON_CreateString("674320f700005d59");
    cJSON *previous_block_hash = cJSON_CreateString("50120119172a610421a6c3011dd330d9df07b63616c2cc1f1cd0020000000000");
    cJSON *coinbase_prefix =
        cJSON_CreateString("01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b");
    cJSON *coinbase_suffix = cJSON_CreateString("ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd745"
                                                "7516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000");
    cJSON *block_version = cJSON_CreateString("01000000");
    cJSON *nbits = cJSON_CreateString("4c86041b");
    cJSON *time = cJSON_CreateString("37221b4d");
    cJSON *forget_previous_job = cJSON_CreateBool(cJSON_True);

    cJSON *params_arr = cJSON_AddArrayToObject(TEST_JSON, "params");
    cJSON_AddItemToArray(params_arr, job_id);
    cJSON_AddItemToArray(params_arr, previous_block_hash);
    cJSON_AddItemToArray(params_arr, coinbase_prefix);
    cJSON_AddItemToArray(params_arr, coinbase_suffix);
    cJSON_AddItemToArray(params_arr, merkle_branches_arr);
    cJSON_AddItemToArray(params_arr, block_version);
    cJSON_AddItemToArray(params_arr, nbits);
    cJSON_AddItemToArray(params_arr, time);
    cJSON_AddItemToArray(params_arr, forget_previous_job);

    // Create the dependent queue
    stratum_to_job_queue = xQueueCreate(10, sizeof(mining_notify *));
}

static void test_teardown() {
    cJSON_Delete(TEST_JSON);
    vQueueDelete(stratum_to_job_queue);
}

TEST_CASE("mining.notify message gets put on the job queue", "[notify_request_handler]") {
    test_setup();
    STRATUM.server_request_handler.notify(TEST_JSON);
    TEST_ASSERT_EQUAL(1, uxQueueMessagesWaiting(stratum_to_job_queue));
    test_teardown();
}

TEST_CASE("mining.notify with empty or missing `params` is not put on the job queue", "[notify_request_handler]") {
    test_setup();

    // Missing `params` key
    cJSON_DeleteItemFromObject(TEST_JSON, "params");
    STRATUM.server_request_handler.notify(TEST_JSON);
    TEST_ASSERT_EQUAL(0, uxQueueMessagesWaiting(stratum_to_job_queue));

    // "params": null
    cJSON_AddNullToObject(TEST_JSON, "params");
    STRATUM.server_request_handler.notify(TEST_JSON);
    TEST_ASSERT_EQUAL(0, uxQueueMessagesWaiting(stratum_to_job_queue));

    // "params": []
    cJSON_DeleteItemFromObject(TEST_JSON, "params");
    cJSON_AddArrayToObject(TEST_JSON, "params");
    STRATUM.server_request_handler.notify(TEST_JSON);
    TEST_ASSERT_EQUAL(0, uxQueueMessagesWaiting(stratum_to_job_queue));

    test_teardown();
}

TEST_CASE("mining.notify invalid prev block hash is not put on the job queue", "[notify_request_handler]") {
    test_setup();
    cJSON *params_arr = cJSON_GetObjectItem(TEST_JSON, "params");
    cJSON *not_a_block_hash = cJSON_CreateString("not-a-block-hash");
    cJSON_ReplaceItemInArray(params_arr, 1, not_a_block_hash);

    STRATUM.server_request_handler.notify(TEST_JSON);
    TEST_ASSERT_EQUAL(0, uxQueueMessagesWaiting(stratum_to_job_queue));
    test_teardown();
}