#ifndef QUEUE_HANDLES_H
#define QUEUE_HANDLES_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t stratum_to_job_queue;
extern QueueHandle_t work_to_asic_queue;

#endif
