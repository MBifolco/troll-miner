#include "wifi_connection.h"
#include "pool.h"
#include "hard_rtc.h"
#include "job.h"
#include "stratum_message.h"
#include "queue_handles.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t stratum_to_job_queue;

void app_main() {
    // Initialize the RTC
    hard_rtc_init();

    // Declare a variable of type rtc_time_t (or hard_rtc_time_t)
    hard_rtc_time_t now;

    //hard_rtc_time_t now;
    hard_rtc_get_time(&now);

    printf("Current time: %02d:%02d:%02d\n", now.hours, now.minutes, now.seconds);

    // Set a new time
    //hard_rtc_time_t new_time = { .seconds = 0, .minutes = 30, .hours = 12, .day = 3, .date = 28, .month = 11, .year = 24 };
    //hard_rtc_set_time(&new_time);

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
        configMAX_PRIORITIES - 1,
        NULL
    );
}
