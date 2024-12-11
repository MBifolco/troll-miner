#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "job.h"
#include "mock_pool.h"
#include "pool.h"
#include "queue_handles.h"
#include "sdkconfig.h"
#include "stratum_message.h"
#include "wifi_connection.h"

QueueHandle_t stratum_to_job_queue;

void app_main() {
    if (!CONFIG_MOCK_POOL) {
        // Initialize Wi-Fi
        wifi_init();

        // Wait for connection
        wifi_wait_for_connection();
    } else {
        ESP_LOGI("AppInit", "%s", MOCK_ENABLED_BANNER);
    }

    // Initialize the pool component
    pool_init();

    // Create the queues
    stratum_to_job_queue = xQueueCreate(10, sizeof(mining_notify *));

    if (stratum_to_job_queue == NULL) {
        printf("Failed to create queues!\n");
        return;
    }

    // Start the pool task - pick the one we need: mock or not?
    TaskFunction_t pt = CONFIG_MOCK_POOL ? mock_pool_task : pool_task;
    xTaskCreate(pt, "pool_task", 4096, NULL, 5, NULL);

    xTaskCreate((TaskFunction_t)job_task, "job_task", 4096, NULL, 5, NULL);
}
