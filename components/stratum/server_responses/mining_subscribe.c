#include "cJSON.h"
#include "esp_log.h"
#include "pool.h"
#include "stratum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "STRATUM_SERVER_RESPONSE";

void subscribe_response_handler(cJSON *json) {
    ESP_LOGD(TAG, "mining.subscribe response handler: %s", cJSON_Print(json));

    cJSON *error = cJSON_GetObjectItem(json, "error");
    if (error != NULL && !cJSON_IsNull(error)) {
        ESP_LOGE(TAG, "mining.subscribe returned an error: %s", cJSON_Print(error));
        return;
    }

    cJSON *result = cJSON_GetObjectItem(json, "result");
    if (result == NULL) {
        return;
    }
    if (cJSON_GetArraySize(result) < 3) {
        ESP_LOGE(TAG, "mining.subscribe response `result` key is missing values");
        return;
    }

    char *extranonce1 = cJSON_GetArrayItem(result, 1)->valuestring;
    int extranonce2_len = cJSON_GetArrayItem(result, 2)->valueint;
    STRATUM.session.extranonce2 = malloc(extranonce2_len);
    if (STRATUM.session.extranonce2 == NULL) {
        ESP_LOGE(TAG, "Unable to allocate memory for extranonce2");
        return;
    }

    // Clear to modify POOL_SESSION
    // Be explicit and initialize everything to zero
    memset(STRATUM.session.extranonce2, 0, STRATUM.session.extranonce2_len);
    STRATUM.session.extranonce1 = extranonce1;
    STRATUM.session.extranonce2_len = extranonce2_len;

    ESP_LOGD(TAG, "STRATUM.session.extranonce1: %s", STRATUM.session.extranonce1);
    ESP_LOGD(TAG, "STRATUM.session.extranonce2_len: %d", STRATUM.session.extranonce2_len);
    for (int i = 0; i < STRATUM.session.extranonce2_len; i++) {
        ESP_LOGD(TAG, "STRATUM.session.extranonce2[%d]: 0x%02x", i, STRATUM.session.extranonce2[i]);
    }
}