#include "stratum_message.h"
#include "esp_log.h"
#include "jobs.h"
/*
For processing stratum messages: On same ESP32 as pool connection
*/

// this would be the one function that every other component would call
// depending on if it's processed on this cpu, external esp32, fpga, etc
// it would do different things
// we'll need to set up a config so the make files compile the right file based on the target
// or we can do a big switch !
void process_stratum_message(const char *message) {
    ESP_LOGI("STRATUM", "Received message: %s", message);
    receive_stratum_message(message);
}