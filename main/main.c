#include "asic.h"
#include "asic_response_task.h"
#include "esp_log.h"
#include "job.h"
#include "pool.h"
#include "queue_handles.h"
#include "stratum_message.h"
#include "wifi_connection.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "job.h"
#include "mock_pool.h"
#include "pool.h"
#include "queue_handles.h"
#include "sdkconfig.h"
#include "serial.h"
#include "stratum_message.h"
#include "wifi_connection.h"
#include "bm1366.h"
#include <unistd.h>
QueueHandle_t stratum_to_job_queue;
QueueHandle_t work_to_asic_queue;

void app_main() {
#ifdef CONFIG_MOCK_POOL
    TaskFunction_t pt = mock_pool_task;
    ESP_LOGI("AppInit", "%s", MOCK_ENABLED_BANNER);
#else
    TaskFunction_t pt = pool_task;

    // Initialize Wi-Fi
    wifi_init();

    // Wait for connection
    wifi_wait_for_connection();
#endif
    // Initialize the pool component
    /*
    pool_init();

    // Create the queues
    stratum_to_job_queue = xQueueCreate(10, sizeof(mining_notify *));
    work_to_asic_queue = xQueueCreate(10, BLOCK_HEADER_SIZE);

    if (stratum_to_job_queue == NULL || work_to_asic_queue == NULL) {
        printf("Failed to create queues!\n");
        return;
    }
    */
    // initialize serial connection to asic
    ESP_LOGI("AppInit", "Initializing serial connection to ASIC");
    SERIAL_init();

    // Initialize the ASIC
    ESP_LOGI("AppInit", "Initializing ASIC");
    asic_init(1, 1, 1);
    /*
    // Start the pool task to listen to pool
    xTaskCreate(pt, "pool_task", 4096, NULL, 5, NULL);

    // Start the job task to listen to stratum messages
    xTaskCreate((TaskFunction_t)job_task, "job_task", 4096, NULL, 5, NULL);

    // Start the asic task to listen to work and send to asic
    xTaskCreate((TaskFunction_t)asic_task, "asic_task", 4096, NULL, 5, NULL);
    */

   // Start the asic response task to listen to asic
    xTaskCreate((TaskFunction_t)asic_response_task, "asic_response_task", 4096, NULL, 5, NULL);
}
