#include "wifi_connection.h"
#include "pool.h"
#include "job.h"
#include "stratum_message.h"
#include "queue_handles.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t stratum_to_job_queue;

void app_main() {
    // Initialize Wi-Fi
    wifi_init();

    // Wait for connection
    wifi_wait_for_connection();

    // Initialize the pool component
    pool_init();

    // Create the queues
    stratum_to_job_queue = xQueueCreate(10, sizeof(mining_notify*));

    if (stratum_to_job_queue == NULL) {
        printf("Failed to create queues!\n");
        return;
    }

    // Start the pool task
    xTaskCreate(
        (TaskFunction_t)pool_task,
        "pool_task",
        4096,
        NULL,
        5,
        NULL
    );
    
    xTaskCreate(
        (TaskFunction_t)job_task,
        "job_task",
        4096,
        NULL,
        5,
        NULL
    );
}
