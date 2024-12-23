#include "fayksic.h"
#include "asic.h"
#include "crc.h"
#include "job.h"
#include "serial.h"
#include "utils.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RST_PIN GPIO_NUM_1

#define TYPE_JOB 0x20
#define TYPE_CMD 0x40

#define GROUP_SINGLE 0x00
#define GROUP_ALL 0x10

#define CMD_JOB 0x01
#define CMD_WRITE 0x01
#define CMD_INACTIVE 0x03

#define TICKET_MASK 0x14
#define MISC_CONTROL 0x18

static const char *TAG = "Fayksic";
static uint8_t id = 0;

/// @brief
/// @param ftdi
/// @param header
/// @param data
/// @param len
static void _send(uint8_t header, uint8_t *data, uint8_t data_len, bool debug) {
    ESP_LOGI(TAG, "Sending serialized data");
    packet_type_t packet_type = (header & TYPE_JOB) ? JOB_PACKET : CMD_PACKET;
    uint8_t total_length = (packet_type == JOB_PACKET) ? (data_len + 6) : (data_len + 5);

    // allocate memory for buffer
    unsigned char *buf = malloc(total_length);

    // add the preamble
    buf[0] = 0x55;
    buf[1] = 0xAA;

    // add the header field
    buf[2] = header;

    // add the length field
    buf[3] = (packet_type == JOB_PACKET) ? (data_len + 4) : (data_len + 3);

    // add the data
    memcpy(buf + 4, data, data_len);

    // add the correct crc type
    if (packet_type == JOB_PACKET) {
        uint16_t crc16_total = crc16_false(buf + 2, data_len + 2);
        buf[4 + data_len] = (crc16_total >> 8) & 0xFF;
        buf[5 + data_len] = crc16_total & 0xFF;
    } else {
        buf[4 + data_len] = crc5(buf + 2, data_len + 2);
    }

    // send serial data
    ESP_LOGI(TAG, "Sending %d bytes", total_length);
    prettyHex(buf, total_length);
    printf("\n");
    SERIAL_send(buf, total_length, debug);

    free(buf);
}

void send_work(uint8_t block_header[BLOCK_HEADER_SIZE]) {
    // Local variable to store the work to be sent

    // Update work ID, cycling through values with modulo 128
    id = (id + 8) % 128;
    uint8_t work[WORK_SIZE];

    // TODO: Do id & num_midstates belong "in the front" or "in the back"?
    work[0] = id;
    work[1] = 0x01; // Set the number of midstates (assuming fixed value)
    memcpy(work + 2, block_header, BLOCK_HEADER_SIZE);

    // Log the job being sent
    ESP_LOGI(TAG, "Send Job: %02X", work[0]);
    prettyHex(work, WORK_SIZE);
    printf("\n");
    // Send the work to the ASIC
    _send((TYPE_JOB | GROUP_SINGLE | CMD_WRITE), work, WORK_SIZE, false);
}

void split_uint32_to_bytes(uint32_t value, uint8_t *bytes, uint8_t idx) {
    bytes[idx + 0] = (value >> 24) & 0xFF; // Extract the most significant byte
    bytes[idx + 1] = (value >> 16) & 0xFF; // Extract the second most significant byte
    bytes[idx + 2] = (value >> 8) & 0xFF;  // Extract the second least significant byte
    bytes[idx + 3] = value & 0xFF;         // Extract the least significant byte
}

uint32_t closest_power_of_2_minus_1(uint32_t high, uint32_t low) {
    // If the number is 0, the mask will also be 0
    if (high == 0 && low == 0) {
        return 0;
    }

    // Find the closest power of 2 less than or equal to the number
    unsigned long long power_of_2 = 1;

    // Handle the high 32 bits
    if (high != 0) {
        while (high > 0) {
            power_of_2 <<= 1;
            high >>= 1;
        }
        power_of_2 >>= 1; // Shift back once because we overshot the power of 2
    }

    // Handle the low 32 bits
    if (low != 0) {
        while (low > 0) {
            power_of_2 <<= 1;
            low >>= 1;
        }
        power_of_2 >>= 1; // Shift back once because we overshot the power of 2
    }

    // Subtract 1 from the closest power of 2 to create the mask
    return power_of_2 - 1;
}

// TODO: Find & update send_job_diff refs
void send_job_difficulty(uint32_t difficulty) {
    /**
     * This algorithm produces a target that should be "good enough" for an ESP32 miner.
     * We lose all precision after bit 64, but it keeps up with other algorithms that work
     * with 64-bit integers up to that point. They are better suited to accurately track the
     * full dividend. This was a choice to optimize the target calculations for the 32-bit
     * architecture of the ESP32.
     */
    uint32_t current_32bits = 0;
    uint32_t result[2] = {0, 0};

    // We're only going to get info about the first 2 32-bit chunks, so don't bother w/ the rest
    for (int i = 2; i >= 0; i--) {
        // Get the current chunk
        current_32bits = (current_32bits << 31) | TRUEDIFFONE[i];

        // Store the result
        result[i] = current_32bits / difficulty;

        // Pull out the remainder
        current_32bits = current_32bits - (result[i] * difficulty);
    }

    // 2 for command, 8 for target mask
    uint8_t job_difficulty_mask[10];
    job_difficulty_mask[0] = 0x00;
    job_difficulty_mask[1] = TICKET_MASK;
    uint32_t power_of_2 = closest_power_of_2_minus_1(result[1], result[0]);

    // TODO: Reverse the bits - however, what in what endianness should they be reversed?
    for (int i = 9; i >= 2; i--) {
        job_difficulty_mask[i] = (uint8_t)(power_of_2 & 0xFF);
        power_of_2 >>= 8;
    }

    ESP_LOGI(TAG, "ASIC difficulty mask payload: ");
    for (int i = 0; i < 10; i++) {
        printf("%02x", job_difficulty_mask[i]);
    }
    printf("\n");

    _send((TYPE_CMD | GROUP_ALL | CMD_WRITE), job_difficulty_mask, 10, false);
}