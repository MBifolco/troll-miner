#include "fayksic.h"

#include "crc.h"
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

static const char * TAG = "Fayksic";

static uint8_t asic_response_buffer[SERIAL_BUF_SIZE];

/// @brief
/// @param ftdi
/// @param header
/// @param data
/// @param len
static void _send_FAYKSIC(uint8_t header, uint8_t * data, uint8_t data_len, bool debug)
{
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
    prettyHex((unsigned char *)data, data_len);
    printf("\n");
    char block_header[160];
    to_hex_string(data, block_header, 80);
    printf("Sending block header: %s\n", block_header);

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
    // log first five bytes of the packet
    //ESP_LOG_BUFFER_HEX(TAG, buf, 100);
    SERIAL_send(buf, total_length, debug);

    free(buf);
}

static void _send_chain_inactive(void)
{
    ESP_LOGI(TAG, "Sending chain inactive");
    unsigned char read_address[2] = {0x00, 0x00};
    // send serial data
    _send_FAYKSIC((TYPE_CMD | GROUP_ALL | CMD_INACTIVE), read_address, 2, SERIALTX_DEBUG);
}

static void _set_chip_address(uint8_t chipAddr){}

void set_version_mask(uint32_t version_mask) 
{
    ESP_LOGI(TAG, "Setting version mask to ");
    int versions_to_roll = version_mask >> 13;
    uint8_t version_byte0 = (versions_to_roll >> 8);
    uint8_t version_byte1 = (versions_to_roll & 0xFF); 
    uint8_t version_cmd[] = {0x00, 0xA4, 0x90, 0x00, version_byte0, version_byte1};
    _send_FAYKSIC(TYPE_CMD | GROUP_ALL | CMD_WRITE, version_cmd, 6, SERIALTX_DEBUG);
}

void send_hash_frequency(float target_freq){}

static void do_frequency_ramp_up(){}

static uint8_t _send_init(uint64_t frequency, uint16_t asic_count)
{
    // set version mask
    for (int i = 0; i < 3; i++) {
        set_version_mask(STRATUM_DEFAULT_VERSION_MASK);
    }
    return 1;
}

// reset the FAYKSIC via the RTS line
static void _reset(void)
{
    //gpio_set_level(RST_PIN, 0);

    // delay for 100ms
    //vTaskDelay(100 / portTICK_PERIOD_MS);

    // set the gpio pin high
    //gpio_set_level(RST_PIN, 1);

    // delay for 100ms
    //vTaskDelay(100 / portTICK_PERIOD_MS);
}

uint8_t init(uint64_t frequency, uint16_t asic_count)
{
    ESP_LOGI(TAG, "Initializing FAYKSIC");

    memset(asic_response_buffer, 0, SERIAL_BUF_SIZE);

    esp_rom_gpio_pad_select_gpio(RST_PIN);
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);

    // reset the FAYKSIC
    _reset();

    return _send_init(frequency, asic_count);
}

// Baud formula = 25M/((denominator+1)*8)
// The denominator is 5 bits found in the misc_control (bits 9-13)
int set_default_baud(void)
{
     // default divider of 26 (11010) for 115,749
    unsigned char baudrate[9] = {0x00, MISC_CONTROL, 0x00, 0x00, 0b01111010, 0b00110001}; // baudrate - misc_control
    _send_FAYKSIC((TYPE_CMD | GROUP_ALL | CMD_WRITE), baudrate, 6, BM1366_SERIALTX_DEBUG);
    return 115200;
}

int set_max_baud(void)
{
    return 115200;
}

void set_job_difficulty_mask(int difficulty)
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

    _send_FAYKSIC((TYPE_CMD | GROUP_ALL | CMD_WRITE), job_difficulty_mask, 6, SERIALTX_DEBUG);
}

static uint8_t id = 0;

void send_work(void * pvParameters, bm_job * next_bm_job)
{

    GlobalState * GLOBAL_STATE = (GlobalState *) pvParameters;

    job job;
    id = (id + 8) % 128;
    job.job_id = id;
    job.num_midstates = 0x01;
    memcpy(&job.starting_nonce, &next_bm_job->starting_nonce, 4);
    memcpy(&job.nbits, &next_bm_job->target, 4);
    memcpy(&job.ntime, &next_bm_job->ntime, 4);
    memcpy(job.merkle_root, next_bm_job->merkle_root_be, 32);
    memcpy(job.prev_block_hash, next_bm_job->prev_block_hash_be, 32);
    memcpy(&job.version, &next_bm_job->version, 4);

    if (GLOBAL_STATE->ASIC_TASK_MODULE.active_jobs[job.job_id] != NULL) {
        //this is from the mining module i haven't imported
        //free_bm_job(GLOBAL_STATE->ASIC_TASK_MODULE.active_jobs[job.job_id]);
    }

    GLOBAL_STATE->ASIC_TASK_MODULE.active_jobs[job.job_id] = next_bm_job;

    pthread_mutex_lock(&GLOBAL_STATE->valid_jobs_lock);
    GLOBAL_STATE->valid_jobs[job.job_id] = 1;
    pthread_mutex_unlock(&GLOBAL_STATE->valid_jobs_lock);

    //debug sent jobs - this can get crazy if the interval is short

    ESP_LOGI(TAG, "Send Job: %02X", job.job_id);


    _send_FAYKSIC((TYPE_JOB | GROUP_SINGLE | CMD_WRITE), (uint8_t *)&job, sizeof(job), DEBUG_WORK);
}