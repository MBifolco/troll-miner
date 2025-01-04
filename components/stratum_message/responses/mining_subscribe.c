/**
 * Parse mining_subscribe responses from the startum server
 *
 * Example: {"result":[["mining.notify","677a0966"],"19865d67",8],"id":2,"error":null}
 *
 * Where "result": contains an array of
 *   - An array of messages the client is subscribed to and a job ID
 *   - extranonce1 value
 *   - Byte length of extranonce2
 */

#include "esp_log.h"
#include "pool.h"
#include "responses.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "PROCESS_MINING_SUBSCRIBE_RESPONSE";

void process_mining_subscribe_response(cJSON *json) {
    ESP_LOGI(TAG, "mining.subscribe response");

    cJSON *result = cJSON_GetObjectItem(json, "result");
    POOL_SESSION.extranonce1 = cJSON_GetArrayItem(result, 1)->valuestring;
    POOL_SESSION.extranonce2_len = cJSON_GetArrayItem(result, 2)->valueint;
    POOL_SESSION.extranonce2 = malloc(POOL_SESSION.extranonce2_len);
    if (POOL_SESSION.extranonce2 == NULL) {
        ESP_LOGE(TAG, "Unable to allocate memory for extranonce2");
        return;
    }

    // Be explicit and initialize everything to zero
    memset(POOL_SESSION.extranonce2, 0, POOL_SESSION.extranonce2_len);

    ESP_LOGD(TAG, "POOL_SESSION.extranonce1: %s", POOL_SESSION.extranonce1);
    ESP_LOGD(TAG, "POOL_SESSION.extranonce2_len: %d", POOL_SESSION.extranonce2_len);
    for (int i = 0; i < POOL_SESSION.extranonce2_len; i++) {
        ESP_LOGD(TAG, "POOL_SESSION.extranonce2[%d]: 0x%02x", i, POOL_SESSION.extranonce2[i]);
    }
}
