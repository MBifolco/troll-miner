#include "wifi_connection.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    // Initialize Wi-Fi
    wifi_init();

    // Wait for connection
    wifi_wait_for_connection();

    // Start other tasks or functionality
}
