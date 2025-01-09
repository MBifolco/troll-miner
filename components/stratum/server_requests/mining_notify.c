#include "cJSON.h"
#include "esp_log.h"
#include "job.h"
#include "queue_handles.h"
#include "utils.h"
#include <string.h>

static char *TAG = "STRATUM_SERVER_REQUEST_NOTIFY";

void notify_request_handler(cJSON *json) {
    mining_notify *mining_notify = malloc(sizeof(*mining_notify));
    if (!mining_notify) {
        ESP_LOGE(TAG, "Failed to allocate memory for mining_notify");
        return;
    }

    cJSON *params = cJSON_GetObjectItem(json, "params");
    if (params == NULL || cJSON_IsNull(params)) {
        ESP_LOGE(TAG, "missing `params`");
        return;
    }

    if (cJSON_GetArraySize(params) < 9) {
        ESP_LOGE(TAG, "`params` is not the right size. Expected at least 10, got %d", cJSON_GetArraySize(params));
        return;
    }

    cJSON *jobIdItem = cJSON_GetArrayItem(params, 0);
    if (jobIdItem && cJSON_IsString(jobIdItem)) {
        mining_notify->job_id = strdup(jobIdItem->valuestring);
        if (!mining_notify->job_id) {
            ESP_LOGE(TAG, "Failed to allocate memory for job ID");
            free(mining_notify);
            return;
        }
        ESP_LOGI(TAG, "Parsed Job ID: %s", mining_notify->job_id);
    } else {
        ESP_LOGE(TAG, "Job ID is missing or not a string");
        mining_notify->job_id = strdup("INVALID_JOB_ID");
    }

    // set prev block hash
    int res = hex2bin(cJSON_GetArrayItem(params, 1)->valuestring, mining_notify->prev_block_hash, HASH_SIZE);
    if (res != HASH_SIZE) {
        ESP_LOGE(TAG, "prev_block_hash was not the right size. Expected: %d; got %d", HASH_SIZE, res);
        goto end_from_job_id;
    }

    // set coinbase prefix & suffix
    mining_notify->coinbase_prefix = strdup(cJSON_GetArrayItem(params, 2)->valuestring);
    if (!mining_notify->coinbase_prefix) {
        ESP_LOGE(TAG, "Failed to allocate memory for coinbase_prefix");
        goto end_from_prefix;
    }

    mining_notify->coinbase_suffix = strdup(cJSON_GetArrayItem(params, 3)->valuestring);
    if (!mining_notify->coinbase_suffix) {
        ESP_LOGE(TAG, "Failed to allocate memory for coinbase_suffix");
        goto end_from_prefix;
    }

    ESP_LOGD(TAG, "Job ID after allocations: %s (Address: %p)", mining_notify->job_id, (void *)mining_notify->job_id);

    // Set merkle branches
    cJSON *merkle_branch = cJSON_GetArrayItem(params, 4);
    size_t n_merkle_branches = cJSON_GetArraySize(merkle_branch);
    if (n_merkle_branches > MAX_MERKLE_BRANCHES) {
        printf("Too many Merkle branches.\n");
        goto end_from_suffix;
    }

    // TODO: Special case of 0 merkle branches provided

    mining_notify->n_merkle_branches = n_merkle_branches;
    mining_notify->merkle_branches = malloc(HASH_SIZE * n_merkle_branches);
    if (!mining_notify->merkle_branches) {
        ESP_LOGE(TAG, "Failed to allocate memory for merkle branches");
        goto end_from_suffix;
    }
    // set block version
    mining_notify->version = strtoul(cJSON_GetArrayItem(params, 5)->valuestring, NULL, 16);

    // set network difficulty
    mining_notify->nbits = strtoul(cJSON_GetArrayItem(params, 6)->valuestring, NULL, 16);

    // set ntime
    mining_notify->time = strtoul(cJSON_GetArrayItem(params, 7)->valuestring, NULL, 16);

    for (size_t i = 0; i < n_merkle_branches; i++) {
        const char *hex_str = cJSON_GetArrayItem(merkle_branch, i)->valuestring;
        if (strlen(hex_str) != HASH_SIZE * 2) {
            ESP_LOGE(TAG, "Invalid Merkle branch length at index %zu: %zu", i, strlen(hex_str));
            goto end_from_branches;
            return;
        }
        ESP_LOGI(TAG, "Converting Merkle branch %zu: %s", i, hex_str);
        hex2bin(hex_str, mining_notify->merkle_branches + HASH_SIZE * i, HASH_SIZE);
    }

    // havent figured out what these do yet
    // params can be varible length
    // int paramsLength = cJSON_GetArraySize(params);
    // int value = cJSON_IsTrue(cJSON_GetArrayItem(params, paramsLength - 1));
    // message->should_abandon_work = value;

    if (xQueueSend(stratum_to_job_queue, &mining_notify, portMAX_DELAY) != pdPASS) {
        printf("Failed to send mining_notify to queue!\n");
        goto end_from_branches;
    }
    ESP_LOGI(TAG, "sent Job ID: %s", mining_notify->job_id);

    check_queue_items();
    goto end;

end_from_branches:
    free(mining_notify->merkle_branches);
end_from_suffix:
    free(mining_notify->coinbase_suffix);
end_from_prefix:
    free(mining_notify->coinbase_prefix);
end_from_job_id:
    free(mining_notify->job_id);
    free(mining_notify);
end:
}