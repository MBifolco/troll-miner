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
    // Send the work to the ASIC
    _send((TYPE_JOB | GROUP_SINGLE | CMD_WRITE), work, WORK_SIZE, false);
}

void send_job_difficulty(int difficulty)
{

    // Default mask of 256 diff
    unsigned char job_difficulty_mask[9] = {0x00, TICKET_MASK, 0b00000000, 0b00000000, 0b00000000, 0b11111111};

    // The mask must be a power of 2 so there are no holes
    // Correct:  {0b00000000, 0b00000000, 0b11111111, 0b11111111}
    // Incorrect: {0b00000000, 0b00000000, 0b11100111, 0b11111111}
    // (difficulty - 1) if it is a pow 2 then step down to second largest for more hashrate sampling
    difficulty = _largest_power_of_two(difficulty) - 1;

    // convert difficulty into char array
    // Ex: 256 = {0b00000000, 0b00000000, 0b00000000, 0b11111111}, {0x00, 0x00, 0x00, 0xff}
    // Ex: 512 = {0b00000000, 0b00000000, 0b00000001, 0b11111111}, {0x00, 0x00, 0x01, 0xff}
    for (int i = 0; i < 4; i++) {
        char value = (difficulty >> (8 * i)) & 0xFF;
        // The char is read in backwards to the register so we need to reverse them
        // So a mask of 512 looks like 0b00000000 00000000 00000001 1111111
        // and not 0b00000000 00000000 10000000 1111111

        job_difficulty_mask[5 - i] = _reverse_bits(value);
    }

    ESP_LOGI(TAG, "Setting job ASIC mask to %d", difficulty);

    _send((TYPE_CMD | GROUP_ALL | CMD_WRITE), job_difficulty_mask, 6, false);
}