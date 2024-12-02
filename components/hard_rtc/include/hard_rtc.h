#ifndef HARD_RTC_COMPONENT_H
#define HARD_RTC_COMPONENT_H

#include <stdint.h>

// Struct to represent DS3231 time
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;    // Day of the week (1-7)
    uint8_t date;   // Day of the month (1-31)
    uint8_t month;  // Month (1-12)
    uint8_t year;   // Year (0-99)
} hard_rtc_time_t;

// Initialize the I2C driver and DS3231 module
void hard_rtc_init();

// Get the current time from the DS3231
void hard_rtc_get_time(hard_rtc_time_t *time);

// Set the current time on the DS3231
void hard_rtc_set_time(const hard_rtc_time_t *time);

#endif // HARD_RTC_COMPONENT_H