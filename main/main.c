#include "wifi_connection.h"
#include "pool.h"
#include "job.h"
#include "stratum_message.h"
#include "queue_handles.h"
#include "asic.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t stratum_to_job_queue;
QueueHandle_t work_to_asic_queue;

void app_main() {
    // Initialize Wi-Fi
    wifi_init();

    // Wait for connection
    wifi_wait_for_connection();

    // Initialize the pool component
    pool_init();

    // Create the queues
    stratum_to_job_queue = xQueueCreate(10, sizeof(mining_notify*));
    work_to_asic_queue = xQueueCreate(10, sizeof(mining_work*));

    if (stratum_to_job_queue == NULL || work_to_asic_queue == NULL) {
        printf("Failed to create queues!\n");
        return;
    }

    // Start the pool task to listen to pool
    xTaskCreate(
        (TaskFunction_t)pool_task,
        "pool_task",
        4096,
        NULL,
        5,
        NULL
    );
    
    // Start the job task to listen to stratum messages
    xTaskCreate(
        (TaskFunction_t)job_task,
        "job_task",
        4096,
        NULL,
        5,
        NULL
    );

    // Start the asic task to listen to work and send to asic
    xTaskCreate(
        (TaskFunction_t)asic_task,
        "asic_task",
        4096,
        NULL,
        5,
        NULL
    );
}
