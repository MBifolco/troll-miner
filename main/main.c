#include "wifi_connection.h"
#include "pool.h"
//#include "ntp-time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    // Initialize Wi-Fi
    wifi_init();

    // Wait for connection
    wifi_wait_for_connection();

    // Obtain the current time
    //obtain_time();

    // Initialize the pool component
    pool_init();

    // Start the pool task
    xTaskCreate(
        (TaskFunction_t)pool_task,
        "PoolTask",
        4096,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL
    );
}
