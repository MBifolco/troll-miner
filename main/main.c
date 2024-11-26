#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define POOL_CONNECTED_BIT BIT1

static EventGroupHandle_t connection_event_group;

static const char * TAG = "application_entry";

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected. Reconnecting...");
        xEventGroupClearBits(connection_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected to Wi-Fi, IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(connection_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init() {
        ESP_LOGI(TAG, "Initializing Wi-Fi...");

    // Initialize the TCP/IP stack
    esp_netif_init();

    // Create the default event loop if it hasn't been created
    esp_event_loop_create_default();

    // Create a Wi-Fi station interface
    esp_netif_create_default_wifi_sta();

    // Initialize the Wi-Fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi: %s", esp_err_to_name(ret));
        return;
    }

    // Register event handlers for Wi-Fi and IP events
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);

    // Configure Wi-Fi with SSID and password
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);  // Set the mode to station
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);  // Apply the configuration
    esp_wifi_start();  // Start the Wi-Fi driver
}

void app_main(void)
{   
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Erase NVS if the partition is full or outdated
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    connection_event_group = xEventGroupCreate();

    // wait for connection to the Wi-Fi network
    wifi_init();

    // and then start the pool connection task
    //xTaskCreate(pool_connection_task, "PoolConnectionTask", 4096, NULL, 5, NULL);
}