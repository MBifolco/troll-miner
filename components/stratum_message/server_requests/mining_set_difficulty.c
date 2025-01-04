#include "cJSON.h"
#include "fayksic.h"
#include "pool.h"

void set_difficulty_request_handler(cJSON *json) {
    cJSON *params = cJSON_GetObjectItem(json, "params");
    if (params == NULL) {
        return;
    }

    cJSON *diff = cJSON_GetArrayItem(params, 0);
    if (diff == NULL) {
        return;
    }

    POOL_SESSION.difficulty = (uint32_t)diff->valueint;

    /**
     * TODO: This should be "configurable", i.e. today we only send to fayksic; tomorrow - other ASICs
     * TODO: This assumes that we'll have access to the ASIC on this particular ESP32
     */
    send_job_difficulty(POOL_SESSION.difficulty);
}
