#include "job.h"
#include "esp_log.h"


void receive_stratum_message(const char *message) {
    ESP_LOGI("STRATUM", "Received message: %s", message);
}