#include "asic.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "job.h"
#include "mock_pool.h"
#include "pool.h"
#include "queue_handles.h"
#include "sdkconfig.h"
#include "serial.h"
#include "stratum.h"
#include "wifi_connection.h"

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
    pool_init();

    // Assign function pointers for stratum object
    stratum_init();

    // Create the queues
    stratum_to_job_queue = xQueueCreate(10, sizeof(mining_notify *));
    work_to_asic_queue = xQueueCreate(10, BLOCK_HEADER_SIZE);

    if (stratum_to_job_queue == NULL || work_to_asic_queue == NULL) {
        printf("Failed to create queues!\n");
        return;
    }

    // initialize serial connection to asic
    SERIAL_init();

    /**
     * TODO: Is there some start/warm up signals that could be sent to the ASIC?
     * No point in doing anything else until we know they're available and ready to work
     */

    // Start the pool task to listen to pool
    xTaskCreate(pt, "pool_task", 4096, NULL, 5, NULL);

    // Start the job task to listen to stratum messages
    xTaskCreate((TaskFunction_t)job_task, "job_task", 4096, NULL, 5, NULL);

    // Start the ASIC task to listen to work and send to ASIC
    xTaskCreate((TaskFunction_t)asic_request_task, "asic_request_task", 4096, NULL, 5, NULL);

    // Start the ASIC result task to listen for ASIC results...
    xTaskCreate((TaskFunction_t)asic_result_listener_task, "asic_result_listener_task", 4096, NULL, 5, NULL);
}
