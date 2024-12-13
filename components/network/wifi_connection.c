#include "wifi_connection.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "WiFi";
static EventGroupHandle_t wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

// Event handler for Wi-Fi events
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected. Retrying...");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected to Wi-Fi, IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Initialize Wi-Fi
void wifi_init() {
    ESP_LOGI(TAG, "Initializing Wi-Fi...");

    // Initialize NVS
    esp_err_t ret_nvs = nvs_flash_init();
    if (ret_nvs == ESP_ERR_NVS_NO_FREE_PAGES || ret_nvs == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Initialize the TCP/IP stack
    esp_netif_init();

    // Create the default event loop if it hasn't been created
    esp_event_loop_create_default();

    // Create a Wi-Fi station interface
    esp_netif_create_default_wifi_sta();

    // Initialize the Wi-Fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret_wifi = esp_wifi_init(&cfg);
    if (ret_wifi != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi: %s", esp_err_to_name(ret_wifi));
        return;
    }

    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create Wi-Fi event group!");
    }

    // Register event handlers for Wi-Fi and IP events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // Configure Wi-Fi with SSID and password
    wifi_config_t wifi_config = {
        .sta =
            {
                .ssid = CONFIG_WIFI_SSID,
                .password = CONFIG_WIFI_PASSWORD,
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };

    ESP_LOGI(TAG, "Connecting to Wi-Fi SSID %s...", wifi_config.sta.ssid);
    ESP_LOGI(TAG, "Password: %s", wifi_config.sta.password);

    strncpy((char *)wifi_config.sta.ssid, CONFIG_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, CONFIG_WIFI_PASSWORD, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Wait for Wi-Fi to connect
void wifi_wait_for_connection() {
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi connected.");
}
