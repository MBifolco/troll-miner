#include "pool.h"
#include "stratum_message.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>
#include <arpa/inet.h>
#include <netdb.h>

static const char *TAG = "Pool";

static int pool_socket = -1;
static EventGroupHandle_t pool_event_group;
static const int POOL_CONNECTED_BIT = BIT0;
// set incrementable message id
static int message_id = 1;
// Function to connect to the pool
static int pool_connect() {
    ESP_LOGI(TAG, "Connecting to pool at %s:%d", CONFIG_POOL_PRIMARY_URL, CONFIG_POOL_PRIMARY_PORT);

    // Create a socket
    pool_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (pool_socket < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return -1;
    }


    // Resolve hostname to IP address
    struct addrinfo hints, *res, *p;
    char ip_address[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Use AF_INET for IPv4; change to AF_UNSPEC for IPv4/IPv6
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(CONFIG_POOL_PRIMARY_URL, NULL, &hints, &res);
    if (status != 0) {
        ESP_LOGE(TAG, "Failed to resolve hostname: %s", getaddrinfo_error(status));
        close(pool_socket);
        return -1;
    }

    // Loop through the results to find an address to connect to
    for (p = res; p != NULL; p = p->ai_next) {
        // Convert the IP to a string for logging
        void *addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
        inet_ntop(p->ai_family, addr, ip_address, sizeof(ip_address));
        ESP_LOGI(TAG, "Resolved IP: %s", ip_address);

        // Set up the server address
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(CONFIG_POOL_PRIMARY_PORT);
        memcpy(&server_addr.sin_addr, addr, sizeof(server_addr.sin_addr));

        // Attempt to connect
        if (connect(pool_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            ESP_LOGI(TAG, "Connected to pool at %s:%d", ip_address, CONFIG_POOL_PRIMARY_PORT);
            freeaddrinfo(res); // Free address info after successful connection
            xEventGroupSetBits(pool_event_group, POOL_CONNECTED_BIT);

            // configure version rolling
            configure_version_rolling();

            // subscribe to pool
            subscribe_to_pool();

            // authenticate with pool
            authenticate_with_pool();

            // suggest difficulty
            suggest_difficulty();
            return 0;
        } else {
            ESP_LOGE(TAG, "Failed to connect to %s", ip_address);
        }
    }
     // Clean up if connection fails
    ESP_LOGE(TAG, "Failed to connect to any resolved IP addresses");
    freeaddrinfo(res);
    close(pool_socket);
    pool_socket = -1;
    return -1;    
}


// Function to receive data from the pool
static int pool_receive(char *buffer, size_t buffer_len) {
    if (pool_socket < 0) {
        ESP_LOGE(TAG, "Socket not connected");
        return -1;
    }

    // in esp-miner they allocate butter dynamically - STRATUM_V1_initialize_buffer
    int bytes_received = recv(pool_socket, buffer, buffer_len - 1, 0);
    if (bytes_received < 0) {
        ESP_LOGE(TAG, "Failed to receive data");
    } else if (bytes_received == 0) {
        ESP_LOGE(TAG, "Pool connection closed by server");
        return -1; // Connection closed
    } else {
        buffer[bytes_received] = '\0'; // Null-terminate the string

        // Split messages by newline
        char *message_start = buffer;
        char *newline = NULL;

        while ((newline = strchr(message_start, '\n')) != NULL) {
            *newline = '\0'; // Replace newline with null terminator

            ESP_LOGI(TAG, "Received message: %s", message_start);
            // Process the individual message
            process_message(message_start);

            // Move to the next message
            message_start = newline + 1;
        }

        // Handle incomplete message (if any)
        if (*message_start != '\0') {
            ESP_LOGW(TAG, "Partial message received: %s", message_start);
            // Optionally, handle partial messages or buffer them
        }
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
        // Increment message id
        message_id++;
        ESP_LOGI(TAG, "Sent message: %s", message);
    }
    return bytes_sent;
}

int configure_version_rolling() {
    // Example: Send a version rolling message
    //const char *version_rolling_msg = "{\"id\": %d, \"method\": \"mining.configure\", \"params\": [{\"version-rolling.mask\": \"0x0000000f\", \"version-rolling.min-difficulty\": 1, \"version-rolling.max-difficulty\": 1, \"version-rolling.threshold\": 1}]}\n";
    char version_rolling_msg[1024];
    sprintf(version_rolling_msg,
            "{\"id\": %d, "
            "\"method\": \"mining.configure\", "
            "\"params\": [[\"version-rolling\"], {\"version-rolling.mask\": \"ffffffff\"}]}\n",
            message_id);
    
    
    return pool_send(version_rolling_msg);
}

int subscribe_to_pool() {
    // Example: Send a subscription message
    //const char *subscribe_msg = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": []}\n";
    char subscribe_msg[1024];
    sprintf(subscribe_msg, "{\"id\": %d, \"method\": \"mining.subscribe\", \"params\": [\"trollminer\"]}\n", message_id);
    return pool_send(subscribe_msg);
}

int authenticate_with_pool() {
    // Example: Send an authentication message
    //const char *auth_msg = "{\"id\": 2, \"method\": \"mining.authorize\", \"params\": [\"username\", \"password\"]}\n";
    char auth_msg[1024];
    sprintf(auth_msg, "{\"id\": %d, \"method\": \"mining.authorize\", \"params\": [\"%s\", \"%s\"]}\n", message_id, CONFIG_POOL_USER, CONFIG_POOL_PW);
    return pool_send(auth_msg);
}

int suggest_difficulty() {
    // Example: Send a difficulty suggestion message
    //const char *suggest_difficulty_msg = "{\"id\": 3, \"method\": \"mining.suggest_difficulty\", \"params\": [1]}\n";
    char suggest_difficulty_msg[1024];
    sprintf(suggest_difficulty_msg, "{\"id\": %d, \"method\": \"mining.suggest_difficulty\", \"params\": [1000]}\n", message_id);
    return pool_send(suggest_difficulty_msg);
}
// Persistent task for managing the pool connection
void pool_task() {
    char *buffer = malloc(2048);

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
        int bytes_received = pool_receive(buffer, 2048);
        if (bytes_received < 0) {
            ESP_LOGE(TAG, "Connection lost. Reconnecting...");
            pool_disconnect();
            vTaskDelay(pdMS_TO_TICKS(5000)); // Wait before retrying
        }
    }
}


const char* getaddrinfo_error(int err) {
    switch (err) {
        case EAI_AGAIN: return "Temporary failure in name resolution";
        case EAI_BADFLAGS: return "Invalid value for ai_flags";
        case EAI_FAIL: return "Non-recoverable failure in name resolution";
        case EAI_FAMILY: return "ai_family not supported";
        case EAI_MEMORY: return "Memory allocation failure";
        case EAI_NONAME: return "Name or service not known";
        case EAI_SERVICE: return "Service not supported for socket type";
        case EAI_SOCKTYPE: return "ai_socktype not supported";
        default: return "Unknown error";
    }
}