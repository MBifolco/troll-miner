#include "stratum_message.h"
#include "esp_log.h"
#include "fayksic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "pool.h"
#include "queue_handles.h"
#include "stratum.h"
#include "utils.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "STRATUM_MESSAGE";

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

void check_queue_items() {
    UBaseType_t items_in_queue = uxQueueMessagesWaiting(stratum_to_job_queue);

    ESP_LOGI(TAG, "Number of items in the queue: %u", (unsigned int)items_in_queue);
}

stratum_server_request_method get_stratum_server_request_method(const char *method_str) {
    if (strcmp(method_str, "mining.notify") == 0) {
        return MINING_NOTIFY;
    }
    if (strcmp(method_str, "mining.set_difficulty") == 0) {
        return MINING_SET_DIFFICULTY;
    }
    if (strcmp(method_str, "mining.set_version_mask") == 0) {
        return MINING_SET_VERSION_MASK;
    }
    return -1;
}

void parse_server_request(cJSON *json) {
    cJSON *method_json = cJSON_GetObjectItem(json, "method");

    if (cJSON_IsString(method_json)) {
        char *method_str = method_json->valuestring;
        stratum_server_request_method method = get_stratum_server_request_method(method_str);
        switch (method) {
        case MINING_NOTIFY:
            ESP_LOGI(TAG, "mining.notify");
            STRATUM.server_request_handler.notify(json);
            break;
        case MINING_SET_DIFFICULTY:
            ESP_LOGI(TAG, "mining.set_difficulty");
            STRATUM.server_request_handler.set_difficulty(json);
            break;
        case MINING_SET_VERSION_MASK:
            ESP_LOGI(TAG, "TODO: mining.set_version_mask");
            break;
        default:
            ESP_LOGE(TAG, "Unknown method in stratum message: %s", method_json->valuestring);
        }
    } else {
        ESP_LOGE(TAG, "method is not a string: %s", cJSON_Print(json));
    }
}

void parse_server_response(cJSON *json) {
    uint8_t msg_id = cJSON_GetObjectItem(json, "id")->valueint;
    if (!msg_id) {
        ESP_LOGE(TAG, "Stratum server response missing ID: %s", cJSON_Print(json));
        return;
    }

    switch (msg_id) {
    case MINING_SUBSCRIBE_MSG_ID:
        STRATUM.server_response_handler.subscribe(json);
        break;

    default:
        ESP_LOGE(TAG, "Unknown stratum server response for given message ID: %d", msg_id);
    }
}

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