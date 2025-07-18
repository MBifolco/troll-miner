#include "bm1366.h"
#include "asic.h"
#include "crc.h"
#include "job.h"
#include "serial.h"
#include "utils.h"
#include "frequency_transition.h"
#include "common.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bm1366.h"

#define RST_PIN GPIO_NUM_1

#define TYPE_JOB 0x20
#define TYPE_CMD 0x40

#define GROUP_SINGLE 0x00
#define GROUP_ALL 0x10

#define CMD_JOB 0x01
#define CMD_WRITE 0x01
#define CMD_INACTIVE 0x03
#define CMD_SETADDRESS 0x00

#define TICKET_MASK 0x14
#define MISC_CONTROL 0x18

#define CHIP_ID 0x1366
#define CHIP_ID_RESPONSE_LENGTH 11

static const char *TAG = "BM1366";
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

void set_version_mask(uint32_t version_mask) 
{
    int versions_to_roll = version_mask >> 13;
    uint8_t version_byte0 = (versions_to_roll >> 8);
    uint8_t version_byte1 = (versions_to_roll & 0xFF); 
    uint8_t version_cmd[] = {0x00, 0xA4, 0x90, 0x00, version_byte0, version_byte1};
    _send(TYPE_CMD | GROUP_ALL | CMD_WRITE, version_cmd, 6, SERIALTX_DEBUG);
}

static void _send_chain_inactive(void)
{

    unsigned char read_address[2] = {0x00, 0x00};
    // send serial data
    _send((TYPE_CMD | GROUP_ALL | CMD_INACTIVE), read_address, 2, SERIALTX_DEBUG);
}

static void _set_chip_address(uint8_t chipAddr)
{

    unsigned char read_address[2] = {chipAddr, 0x00};
    // send serial data
    _send((TYPE_CMD | GROUP_SINGLE | CMD_SETADDRESS), read_address, 2, SERIALTX_DEBUG);
}

static void get_difficulty_mask(uint16_t difficulty, uint8_t *job_difficulty_mask)
{
    // The mask must be a power of 2 so there are no holes
    // Correct:   {0b00000000, 0b00000000, 0b11111111, 0b11111111}
    // Incorrect: {0b00000000, 0b00000000, 0b11100111, 0b11111111}
    difficulty = _largest_power_of_two(difficulty) - 1;

    job_difficulty_mask[0] = 0x00;
    job_difficulty_mask[1] = 0x14; // TICKET_MASK

    // convert difficulty into char array
    // Ex: 256 = {0b00000000, 0b00000000, 0b00000000, 0b11111111}, {0x00, 0x00, 0x00, 0xff}
    // Ex: 512 = {0b00000000, 0b00000000, 0b00000001, 0b11111111}, {0x00, 0x00, 0x01, 0xff}
    job_difficulty_mask[2] = _reverse_bits((difficulty >> 24) & 0xFF);
    job_difficulty_mask[3] = _reverse_bits((difficulty >> 16) & 0xFF);
    job_difficulty_mask[4] = _reverse_bits((difficulty >>  8) & 0xFF);
    job_difficulty_mask[5] = _reverse_bits( difficulty        & 0xFF);
}

static void send_hash_frequency(float target_freq)
{
    // default 200Mhz if it fails
    unsigned char freqbuf[9] = {0x00, 0x08, 0x40, 0xA0, 0x02, 0x41}; // freqbuf - pll0_parameter
    float newf = 200.0;

    uint8_t fb_divider = 0;
    uint8_t post_divider1 = 0, post_divider2 = 0;
    uint8_t ref_divider = 0;
    float min_difference = 10;

    // refdiver is 2 or 1
    // postdivider 2 is 1 to 7
    // postdivider 1 is 1 to 7 and less than postdivider 2
    // fbdiv is 144 to 235
    for (uint8_t refdiv_loop = 2; refdiv_loop > 0 && fb_divider == 0; refdiv_loop--) {
        for (uint8_t postdiv1_loop = 7; postdiv1_loop > 0 && fb_divider == 0; postdiv1_loop--) {
            for (uint8_t postdiv2_loop = 1; postdiv2_loop < postdiv1_loop && fb_divider == 0; postdiv2_loop++) {
                int temp_fb_divider = round(((float) (postdiv1_loop * postdiv2_loop * target_freq * refdiv_loop) / 25.0));

                if (temp_fb_divider >= 144 && temp_fb_divider <= 235) {
                    float temp_freq = 25.0 * (float) temp_fb_divider / (float) (refdiv_loop * postdiv2_loop * postdiv1_loop);
                    float freq_diff = fabs(target_freq - temp_freq);

                    if (freq_diff < min_difference) {
                        fb_divider = temp_fb_divider;
                        post_divider1 = postdiv1_loop;
                        post_divider2 = postdiv2_loop;
                        ref_divider = refdiv_loop;
                        min_difference = freq_diff;
                        break;
                    }
                }
            }
        }
    }

    if (fb_divider == 0) {
        puts("Finding dividers failed, using default value (200Mhz)");
    } else {
        newf = 25.0 * (float) (fb_divider) / (float) (ref_divider * post_divider1 * post_divider2);
        //printf("final refdiv: %d, fbdiv: %d, postdiv1: %d, postdiv2: %d, min diff value: %f\n", ref_divider, fb_divider, post_divider1, post_divider2, min_difference);

        freqbuf[3] = fb_divider;
        freqbuf[4] = ref_divider;
        freqbuf[5] = (((post_divider1 - 1) & 0xf) << 4) + ((post_divider2 - 1) & 0xf);

        if (fb_divider * 25 / (float) ref_divider >= 2400) {
            freqbuf[2] = 0x50;
        }
    }

    _send((TYPE_CMD | GROUP_ALL | CMD_WRITE), freqbuf, 6, SERIALTX_DEBUG);

    ESP_LOGI(TAG, "Setting Frequency to %.2fMHz (%.2f)", target_freq, newf);
}

static void do_frequency_ramp_up(float target_frequency) {
    ESP_LOGI(TAG, "Ramping up frequency to %.2f MHz", target_frequency);
    do_frequency_transition(target_frequency, send_hash_frequency, 1366);
}

int asic_init()
{
    sleep(3); // Pause for 3 seconds

    ESP_LOGI(TAG, "Set version mask to %d", STRATUM_DEFAULT_VERSION_MASK);
    // set version mask
    for (int i = 0; i < 3; i++) {
        set_version_mask(STRATUM_DEFAULT_VERSION_MASK);
    }

    ESP_LOGI("AppInit", "Reading register 00 on all chips");
    unsigned char init3[7] = {0x55, 0xAA, 0x52, 0x05, 0x00, 0x00, 0x0A};
    SERIAL_send(init3, 7, true);

    int chip_counter = count_asic_chips(1, CHIP_ID, CHIP_ID_RESPONSE_LENGTH);

    if (chip_counter == 0) {
        return 0;
    }


    unsigned char init4[11] = {0x55, 0xAA, 0x51, 0x09, 0x00, 0xA8, 0x00, 0x07, 0x00, 0x00, 0x03};
    SERIAL_send(init4, 11, true);

    unsigned char init5[11] = {0x55, 0xAA, 0x51, 0x09, 0x00, 0x18, 0xFF, 0x0F, 0xC1, 0x00, 0x00};
    SERIAL_send(init5, 11, true);

    ESP_LOGI(TAG, "Sending chain inactive command");
    _send_chain_inactive();

    // split the chip address space evenly
    uint8_t address_interval = (uint8_t) (256 / 1);
    for (uint8_t i = 0; i < 1; i++) {
        //{ 0x55, 0xAA, 0x40, 0x05, 0x00, 0x00, 0x1C };
        ESP_LOGI(TAG, "Setting chip address for chip %d", i);
        _set_chip_address(i * address_interval);
    }

    unsigned char init135[11] = {0x55, 0xAA, 0x51, 0x09, 0x00, 0x3C, 0x80, 0x00, 0x85, 0x40, 0x0C};
    SERIAL_send(init135, 11, true);

    unsigned char init136[11] = {0x55, 0xAA, 0x51, 0x09, 0x00, 0x3C, 0x80, 0x00, 0x80, 0x20, 0x19};
    SERIAL_send(init136, 11, true);

    ESP_LOGI(TAG, "Setting difficult mask");
    uint8_t difficulty_mask[6];
    get_difficulty_mask(1, difficulty_mask);
    _send((TYPE_CMD | GROUP_ALL | CMD_WRITE), difficulty_mask, 6,SERIALTX_DEBUG); 

    unsigned char init138[11] = {0x55, 0xAA, 0x51, 0x09, 0x00, 0x54, 0x00, 0x00, 0x00, 0x03, 0x1D};
    SERIAL_send(init138, 11, true);

    unsigned char init139[11] = {0x55, 0xAA, 0x51, 0x09, 0x00, 0x58, 0x02, 0x11, 0x11, 0x11, 0x06};
    SERIAL_send(init139, 11, true);

    unsigned char init171[11] = {0x55, 0xAA, 0x41, 0x09, 0x00, 0x2C, 0x00, 0x7C, 0x00, 0x03, 0x03};
    SERIAL_send(init171, 11, true);

    for (uint8_t i = 0; i < 1; i++) {
        unsigned char set_a8_register[6] = {i * address_interval, 0xA8, 0x00, 0x07, 0x01, 0xF0};
        _send((TYPE_CMD | GROUP_SINGLE | CMD_WRITE), set_a8_register, 6, SERIALTX_DEBUG);
        unsigned char set_18_register[6] = {i * address_interval, 0x18, 0xF0, 0x00, 0xC1, 0x00};
        _send((TYPE_CMD | GROUP_SINGLE | CMD_WRITE), set_18_register, 6, SERIALTX_DEBUG);
        unsigned char set_3c_register_first[6] = {i * address_interval, 0x3C, 0x80, 0x00, 0x85, 0x40};
        _send((TYPE_CMD | GROUP_SINGLE | CMD_WRITE), set_3c_register_first, 6, SERIALTX_DEBUG);
        unsigned char set_3c_register_second[6] = {i * address_interval, 0x3C, 0x80, 0x00, 0x80, 0x20};
        _send((TYPE_CMD | GROUP_SINGLE | CMD_WRITE), set_3c_register_second, 6, SERIALTX_DEBUG);
        unsigned char set_3c_register_third[6] = {i * address_interval, 0x3C, 0x80, 0x00, 0x82, 0xAA};
        _send((TYPE_CMD | GROUP_SINGLE | CMD_WRITE), set_3c_register_third, 6, SERIALTX_DEBUG);
    }

    //do_frequency_ramp_up(250.0);

    unsigned char set_10_hash_counting[6] = {0x00, 0x10, 0x00, 0x00, 0x15, 0x1C}; //S19XP-Stock Default
    // unsigned char set_10_hash_counting[6] = {0x00, 0x10, 0x00, 0x0F, 0x00, 0x00}; //supposedly the "full" 32bit nonce range
    _send((TYPE_CMD | GROUP_ALL | CMD_WRITE), set_10_hash_counting, 6, SERIALTX_DEBUG);

    unsigned char init795[11] = {0x55, 0xAA, 0x51, 0x09, 0x00, 0xA4, 0x90, 0x00, 0xFF, 0xFF, 0x1C};
    SERIAL_send(init795, 11, true);

    return chip_counter;
}