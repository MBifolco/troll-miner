#include "stratum_message.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "queue_handles.h"
#include "fayksic.h"
#include "utils.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "STRATUM_MESSAGE";

void (*methodHandlers[NUM_METHODS])() = {process_mining_notify, process_mining_set_difficulty, process_mining_set_version_mask};

// this would be the one function that every other component would call
// depending on if it's processed on this cpu, external esp32, fpga, etc
// it would do different things
// we'll need to set up a config so the make files compile the right file based on the target
void process_message(const char *message_json) {
    ESP_LOGI(TAG, "Received message: %s", message_json);
    parse_message(message_json);
}

/*
Functions for processing stratum messages on same ESP32 as pool connection
*/

void process_mining_notify(cJSON *json) {
    mining_notify *mining_notify = malloc(sizeof(*mining_notify));
    if (!mining_notify) {
        ESP_LOGE(TAG, "Failed to allocate memory for mining_notify");
        return;
    }

    ESP_LOGI(TAG, "build notify object to send to job queue");
    cJSON *params = cJSON_GetObjectItem(json, "params");

    // set job id
    ESP_LOGI(TAG, "set job id %s", cJSON_GetArrayItem(params, 0)->valuestring);

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
        ESP_LOGE(TAG, "Failed to allocate memory for prev_block_hash");
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

    ESP_LOGI(TAG, "Job ID after allocations: %s (Address: %p)", mining_notify->job_id, (void *)mining_notify->job_id);

    // Set merkle branches
    cJSON *merkle_branch = cJSON_GetArrayItem(params, 4);
    size_t n_merkle_branches = cJSON_GetArraySize(merkle_branch);
    if (n_merkle_branches > MAX_MERKLE_BRANCHES) {
        printf("Too many Merkle branches.\n");
        goto end_from_suffix;
    }

    mining_notify->n_merkle_branches = n_merkle_branches;
    mining_notify->merkle_branches = malloc(HASH_SIZE * n_merkle_branches);
    if (!mining_notify->merkle_branches) {
        ESP_LOGE(TAG, "Failed to allocate memory for merkle branches");
        goto end_from_suffix;
    }
    // // set block version
    mining_notify->version = strtoul(cJSON_GetArrayItem(params, 5)->valuestring, NULL, 16);

    // // set network difficulty
    mining_notify->nbits = strtoul(cJSON_GetArrayItem(params, 6)->valuestring, NULL, 16);

    // // set ntime
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

void check_queue_items() {
    UBaseType_t items_in_queue = uxQueueMessagesWaiting(stratum_to_job_queue);

    ESP_LOGI(TAG, "Number of items in the queue: %u", (unsigned int)items_in_queue);
}

void process_mining_set_difficulty(cJSON *json) {
    ESP_LOGI(TAG, "build set difficulty object to send to job queue");
    cJSON * params = cJSON_GetObjectItem(json, "params");
    mining_set_difficulty * mining_set_difficulty = malloc(sizeof(mining_set_difficulty));
    mining_set_difficulty->difficulty = cJSON_GetArrayItem(params, 0)->valueint;
    
    send_job_difficulty(mining_set_difficulty->difficulty);
}

void process_mining_set_version_mask(cJSON *json) {
    ESP_LOGI(TAG, "build set version mask object to send to job queue");
    // cJSON * params = cJSON_GetObjectItem(json, "params");
    // mining_set_version_mask * mining_set_version_mask = malloc(sizeof(mining_set_version_mask));
    // mining_set_version_mask->version_mask = strtoul(cJSON_GetArrayItem(params, 0)->valuestring, NULL, 16);
    //  call whatever function will be handling set version mask on asic
}

int parase_server_message_id(cJSON *json) {
    cJSON *id_json = cJSON_GetObjectItem(json, "id");
    int64_t parsed_id = -1;
    if (id_json != NULL && cJSON_IsNumber(id_json)) {
        parsed_id = id_json->valueint;
    } else {
        ESP_LOGE(TAG, "Unable to parse id");
    }
    return parsed_id;
}

void parse_server_request(cJSON *json) {
    ESP_LOGI(TAG, "parse server request");
    cJSON *method_json = cJSON_GetObjectItem(json, "method");
    stratum_server_request_method method = -1;

    // it smells to me like there is something redundant here
    if (cJSON_IsString(method_json)) {
        char *method_str = method_json->valuestring;

        if (strcmp(method_str, "mining.notify") == 0) {
            method = MINING_NOTIFY;
            ESP_LOGI(TAG, "mining.notify");
        } else if (strcmp(method_str, "mining.set_difficulty") == 0) {
            method = MINING_SET_DIFFICULTY;
            ESP_LOGI(TAG, "mining.set_difficulty");
        } else if (strcmp(method_str, "mining.set_version_mask") == 0) {
            method = MINING_SET_VERSION_MASK;
            ESP_LOGI(TAG, "mining.set_version_mask");
        } else {
            ESP_LOGI(TAG, "unhandled method in stratum message: %s", method_json->valuestring);
            // should this return error?
        }
    } else {
        ESP_LOGI(TAG, "method is not a string");
        // should this return error?
    }

    if (method >= 0 && method < NUM_METHODS) {
        methodHandlers[method](json);
    } else {
        printf("Invalid method!\n");
    }
}

void parse_server_response(cJSON *json) { ESP_LOGI(TAG, "parse server response"); }

void parse_message(const char *message_json) {
    // is valid json
    cJSON *json = cJSON_Parse(message_json);
    if (json == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON: %s", message_json);
        return;
    }

    // parse method first - it's existence determines if it's a request or response
    // if it's a request we want to get started immediately
    cJSON *method_json = cJSON_GetObjectItem(json, "method");

    // if there is a method this is a server request
    if (method_json != NULL && cJSON_IsString(method_json)) {
        parse_server_request(json);
        // if none then it's a server response with a result or error
    } else {
        parse_server_response(json);
    }

    // whats this?
    // done:
    cJSON_Delete(json);
}