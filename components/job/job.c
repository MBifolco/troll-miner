#include "job.h"
#include "esp_log.h"

#include "queue_handles.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const char *TAG = "JOB";


void job_task()
{
    ESP_LOGI(TAG, "Mining job task started");
    while (true) {
       
    }
}
