#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stratum_message.h"

void mock_pool_task(void *pvParameters) {
    // TODO: stratum_message.c has no way to process a subscribe server response - needed for extranonce1 + extranonce2 len

    // TODO: Need process_message to return something - how do we know if it was successful or not?
    // See: expriements/README.md
    char *MOCK_MINING_NOTIFY = "{\"id\":null,\"method\":\"mining.notify\",\"params\": "
                               "[\"674320f700005d59\",\"000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250\","
                               "\"01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b\","
                               "\"ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fe"
                               "df9ce467cb9f7c6287078f801df276fdf84ac00000000\",[\"c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff\", "
                               "\"49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e\"],\"00000001\",\"4c86041b\",\"4d1b2237\",true]}\0";
    process_message(MOCK_MINING_NOTIFY);
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}