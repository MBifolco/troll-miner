#include "pool.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>

static const char *TAG = "Pool";

static int pool_socket = -1;
static EventGroupHandle_t pool_event_group;
static const int POOL_CONNECTED_BIT = BIT0;

// Function to connect to the pool
static int pool_connect() {
    ESP_LOGI(TAG, "Connecting to pool at %s:%d", CONFIG_POOL_PRIMARY_URL, CONFIG_POOL_PRIMARY_PORT);

    char host_ip[INET_ADDRSTRLEN] = "45.76.165.182";

    //ESP_LOGI(TAG, "Resolving pool hostname...");
    //struct hostent *primary_dns_addr = gethostbyname(CONFIG_POOL_PRIMARY_URL);
    //if (primary_dns_addr == NULL) {
    //    ESP_LOGD(TAG, "Heartbeat. Failed DNS check for: %s!", CONFIG_POOL_PRIMARY_URL);
    //    return -1;
    //}

    //inet_ntop(AF_INET, (void *)primary_dns_addr->h_addr_list[0], host_ip, sizeof(host_ip));

    //ESP_LOGI(TAG, "Pool IP address: %s", host_ip);

    ESP_LOGI(TAG, "Creating socket...");
    // Create a socket
    pool_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (pool_socket < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return -1;
    }

    // Set up the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CONFIG_POOL_PRIMARY_PORT);
    server_addr.sin_addr.s_addr = inet_addr(host_ip);

    // Connect to the server
    if (connect(pool_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to connect to pool");
        close(pool_socket);
        pool_socket = -1;
        return -1;
    }

    ESP_LOGI(TAG, "Connected to pool");
    xEventGroupSetBits(pool_event_group, POOL_CONNECTED_BIT);
    return 0;
}

// Function to receive data from the pool
static int pool_receive(char *buffer, size_t buffer_len) {
    if (pool_socket < 0) {
        ESP_LOGE(TAG, "Socket not connected");
        return -1;
    }

    int bytes_received = recv(pool_socket, buffer, buffer_len - 1, 0);
    if (bytes_received < 0) {
        ESP_LOGE(TAG, "Failed to receive data");
    } else if (bytes_received == 0) {
        ESP_LOGE(TAG, "Pool connection closed by server");
        return -1; // Connection closed
    } else {
        buffer[bytes_received] = '\0'; // Null-terminate the string
        ESP_LOGI(TAG, "Received message: %s", buffer);
    }
    return bytes_received;
}

// Disconnect from the pool
static void pool_disconnect() {
    if (pool_socket >= 0) {
        close(pool_socket);
        pool_socket = -1;
        xEventGroupClearBits(pool_event_group, POOL_CONNECTED_BIT);
        ESP_LOGI(TAG, "Disconnected from pool");
    }
}

void extract_hostname(const char *stratum_url, char *hostname, size_t hostname_len) {
    const char *prefix = "stratum+tcp://";
    if (strncmp(stratum_url, prefix, strlen(prefix)) == 0) {
        snprintf(hostname, hostname_len, "%s", stratum_url + strlen(prefix));
    } else {
        snprintf(hostname, hostname_len, "%s", stratum_url);  // No prefix, use as-is
    }
}

int resolve_hostname(const char *hostname, struct sockaddr_in *server_addr) {
    struct addrinfo hints = {
        .ai_family = AF_INET,  // Use IPv4
        .ai_socktype = SOCK_STREAM,  // TCP stream sockets
    };
    struct addrinfo *res;
    int err = getaddrinfo(hostname, NULL, &hints, &res);
    if (err != 0 || res == NULL) {
        printf("DNS lookup failed for %s: %d\n", hostname, err);
        return -1;
    }

    // Extract the first resolved address
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    server_addr->sin_addr = addr->sin_addr;  // Copy resolved IP address
    server_addr->sin_family = AF_INET;
    freeaddrinfo(res);
    return 0;
}

// Initialize the pool component
void pool_init() {
    ESP_LOGI(TAG, "Initializing pool component...");
    pool_event_group = xEventGroupCreate();
    if (pool_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create pool event group!");
    }
    ESP_LOGI(TAG, "Pool component initialized");
}

// Function to send a message to the pool
int pool_send(const char *message) {
    if (pool_socket < 0) {
        ESP_LOGE(TAG, "Socket not connected");
        return -1;
    }

    int bytes_sent = send(pool_socket, message, strlen(message), 0);
    if (bytes_sent < 0) {
        ESP_LOGE(TAG, "Failed to send message");
    } else {
        ESP_LOGI(TAG, "Sent message: %s", message);
    }
    return bytes_sent;
}

// Persistent task for managing the pool connection
void pool_task() {
    char buffer[512];

    while (true) {
        if (pool_socket < 0) {
            ESP_LOGI(TAG, "Attempting to connect to pool...");
            if (pool_connect() == 0) {
                ESP_LOGI(TAG, "Connected to pool");

                // Example: Send a subscription message
                //const char *subscribe_msg = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": []}\n";
                //pool_send(subscribe_msg);
            } else {
                ESP_LOGE(TAG, "Failed to connect to pool. Retrying...");
                vTaskDelay(pdMS_TO_TICKS(5000)); // Retry after 5 seconds
                continue;
            }
        }

        // Listen for data
        int bytes_received = pool_receive(buffer, sizeof(buffer));
        if (bytes_received < 0) {
            ESP_LOGE(TAG, "Connection lost. Reconnecting...");
            pool_disconnect();
            vTaskDelay(pdMS_TO_TICKS(5000)); // Wait before retrying
        }
    }
}
