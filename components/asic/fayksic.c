#include "fayksic.h"
#include "crc.h"
#include "serial.h"
//#include "utils.h"

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

static const char * TAG = "Fayksic";
static uint8_t id = 0;

/// @brief
/// @param ftdi
/// @param header
/// @param data
/// @param len
static void _send_work(uint8_t header, uint8_t * data, uint8_t data_len, bool debug)
{
    ESP_LOGI(TAG, "Sending serialized data");
    packet_type_t packet_type = (header & TYPE_JOB) ? JOB_PACKET : CMD_PACKET;
    uint8_t total_length = (packet_type == JOB_PACKET) ? (data_len + 6) : (data_len + 5);

    // allocate memory for buffer
    unsigned char * buf = malloc(total_length);

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

void send_work(job_work * job_work)
{
    // local var
    work work;
    id = (id + 8) % 128;
    work.work_id = id;
    work.num_midstates = 0x01;
    memcpy(&work.starting_nonce, &job_work->starting_nonce, 4);
    memcpy(&work.nbits, &job_work->target, 4);
    memcpy(&work.ntime, &job_work->ntime, 4);
    memcpy(work.merkle_root, job_work->merkle_root_be, 32);
    memcpy(work.prev_block_hash, job_work->prev_block_hash_be, 32);
    memcpy(&work.version, &job_work->version, 4);

    ESP_LOGI(TAG, "Send Job: %02X", work.work_id);


    _send_work((TYPE_JOB | GROUP_SINGLE | CMD_WRITE), (uint8_t *)&work, sizeof(work), false);
}