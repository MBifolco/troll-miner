#ifndef JOB_COMPONENT_H
#define JOB_COMPONENT_H
#include "stratum_message.h"
#include <stdint.h>
//#include "stratum_message.h"


void job_task(void *pvParameters);
void format_mining_notify(const mining_notify *job, char *output, size_t output_size);
#endif // JOB_COMPONENT_H
