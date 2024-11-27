#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <sys/time.h>

void initialize_sntp();
void time_sync_notification_cb(struct timeval *tv);
void obtain_time();

#endif // NTPTIME_COMPONENT_H