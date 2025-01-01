#include "cJSON.h"
#include "esp_log.h"
#include "pool.h"
#include "stratum.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "STRATUM_SERVER_RESPONSE";

void subscribe_response_handler(cJSON *json) {
    ESP_LOGI(TAG, "mining.subscribe response");

    cJSON *result = cJSON_GetObjectItem(json, "result");
    if (result == NULL) {
        return;
    }

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