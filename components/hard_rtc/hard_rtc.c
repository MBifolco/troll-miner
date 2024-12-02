#include "hard_rtc.h"
#include "driver/i2c.h"
#include <stdio.h>
#include "esp_log.h"

#define I2C_MASTER_NUM I2C_NUM_0    // I2C port number
#define I2C_MASTER_SDA_IO 7       // GPIO for SDA
#define I2C_MASTER_SCL_IO 6        // GPIO for SCL
#define I2C_MASTER_FREQ_HZ 100000   // I2C clock frequency

#define DS3231_ADDRESS 0x68         // I2C address of DS3231

static const char *TAG = "RTC";
// Helper functions for BCD conversion
static uint8_t bcd_to_decimal(uint8_t bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

static uint8_t decimal_to_bcd(uint8_t decimal) {
    return ((decimal / 10) << 4) | (decimal % 10);
}

// Initialize the I2C driver
void hard_rtc_init(void) {
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &config);
    i2c_driver_install(I2C_MASTER_NUM, config.mode, 0, 0, 0);
    ESP_LOGI(TAG, "I2C driver initialized");
}

// Read multiple bytes from the DS3231 registers
static void hard_rtc_read_registers(uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

// Write a single byte to the DS3231 register
static void hard_rtc_write_register(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

// Get the current time from the DS3231
void hard_rtc_get_time(hard_rtc_time_t *time) {
    uint8_t data[7];
    hard_rtc_read_registers(0x00, data, 7);

    time->seconds = bcd_to_decimal(data[0]);
    time->minutes = bcd_to_decimal(data[1]);
    time->hours = bcd_to_decimal(data[2]);
    time->day = bcd_to_decimal(data[3]);
    time->date = bcd_to_decimal(data[4]);
    time->month = bcd_to_decimal(data[5] & 0x1F); // Mask for century bit
    time->year = bcd_to_decimal(data[6]);
}

// Set the current time on the DS3231
void hard_rtc_set_time(const hard_rtc_time_t *time) {
    uint8_t data[7] = {
        decimal_to_bcd(time->seconds),
        decimal_to_bcd(time->minutes),
        decimal_to_bcd(time->hours),
        decimal_to_bcd(time->day),
        decimal_to_bcd(time->date),
        decimal_to_bcd(time->month),
        decimal_to_bcd(time->year),
    };

    for (int i = 0; i < 7; i++) {
        hard_rtc_write_register(i, data[i]);
    }
}
