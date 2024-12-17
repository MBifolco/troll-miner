#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stratum_message.h"


// Example messages (replace these with your actual stratum messages)
static const char *messages[] = {
    // Response to minnig.configure - the id of 1 tells us this - its the id we sent with our request
    "{\"result\":{\"version-rolling\":true,\"version-rolling.mask\":\"1fffe000\"},\"id\":1,\"error\":null}",
    
    // Response to mining.subscribe - the id -f 2 tells us this - its the id we sent with our request
    "{\"result\":[[\"mining.notify\",\"677a0966\"]],\"19865d67\",8],\"id\":2,\"error\":null}",
    
    // Request mining notification
    "{\"id\":null,\"method\":\"mining.notify\",\"params\": "
    "[\"674320f700005d59\",\"50120119172a610421a6c3011dd330d9df07b63616c2cc1f1cd0020000000000\","
    "\"01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b\","
    "\"ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fe"
    "df9ce467cb9f7c6287078f801df276fdf84ac00000000\",[\"c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff\", "
    "\"49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e\"],\"01000000\",\"4c86041b\",\"37221b4d\",true]}\0",
    
    // Request set difficulty
    "{\"id\": null, \"method\": \"mining.set_difficulty\", \"params\": [512]}",
};

// Corresponding delays (in milliseconds) between messages
static const int message_delays_ms[] = {
    10, // Delay after the first message
    100, // Delay after the second message
    3000,  // Delay after the third message
    5000  // Delay after the fourth message
};


void mock_pool_task(void *pvParameters) {


    size_t total_messages = sizeof(messages) / sizeof(messages[0]);

    for (size_t i = 0; i < total_messages; i++) {
        process_message((char *)messages[i]);

        if (i < total_messages - 1) {
            // Delay before sending the next message
            vTaskDelay(pdMS_TO_TICKS(message_delays_ms[i]));
        } else {
            // After the last message, you could either exit or loop again
            // Here we just delay and then break.
            vTaskDelay(pdMS_TO_TICKS(2000));
            break;
        }
    }

    // If you want this task to continue running and perhaps send messages again,
    // you could loop back or handle logic accordingly.
    // For now, let’s just keep the task alive with a long delay or suspend it.
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
