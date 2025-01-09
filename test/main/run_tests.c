#include "../../main/queue_handles.h"
#include "freertos/queue.h"
#include "unity.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * These are defined in troll-miner's main.c, which we don't really have
 * access to or want to...so we have to make these globals available for unit
 * tests ourselves
 *
 * TODO: These should probably be imported somewhere in troll miner so we can
 * follow suit here, instead of having to redefine all globals
 */
QueueHandle_t stratum_to_job_queue;
QueueHandle_t work_to_asic_queue;

static void print_banner(const char *text);

void app_main(void) {
    /* These are the different ways of running registered tests.
     * In practice, only one of them is usually needed.
     *
     * UNITY_BEGIN() and UNITY_END() calls tell Unity to print a summary
     * (number of tests executed/failed/ignored) of tests executed between these calls.
    print_banner("Executing one test by its name");
    UNITY_BEGIN();
    unity_run_test_by_name("Mean of an empty array is zero");
    UNITY_END();

    print_banner("Running tests with [mean] tag");
    UNITY_BEGIN();
    unity_run_tests_by_tag("[mean]", false);
    UNITY_END();

    print_banner("Running tests without [fails] tag");
    UNITY_BEGIN();
    unity_run_tests_by_tag("[fails]", true);
    UNITY_END();
     */

    print_banner("Running all the registered tests");
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    /* This function will not return, and will be busy waiting for UART input.
     * Make sure that task watchdog is disabled if you use this function.
     */
    // print_banner("Starting interactive test menu");
    // unity_run_menu();
}

static void print_banner(const char *text) { printf("\n#### %s #####\n\n", text); }