#include "../../build/config/sdkconfig.h" // Manually reference build sdkconfig.h - Unity + ESP point to some test default
#include "diff_utils.h"
#include "esp_log.h"
#include "unity.h"
#include "unity_test_runner.h"

static const char *TAG = "DIFF_UTILS_TESTS";

TEST_CASE("target mask of 0 returns 0", "[generate_target_mask]") {
    uint32_t input[2] = {0, 0};

    // Set nonsense for the target_mask, make sure we get back 0
    uint32_t target_mask[2] = {100, 100};
    uint32_t expected_result[2] = {0, 0};

    generate_target_mask(input, target_mask);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_result, target_mask, 2);
}

TEST_CASE("target mask of all 1s returns next lowest value of all 1s", "[generate_target_mask]") {
    uint32_t input[2] = {0xFFFFFFFF, 0xFFFFFFFF};
    uint32_t target_mask[2];
    uint32_t expected_result[2] = {0x7FFFFFFF, 0xFFFFFFFF};

    generate_target_mask(input, target_mask);
    ESP_LOGD(TAG, "Result: 0x%08lx%08lx", target_mask[0], target_mask[1]);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_result, target_mask, 2);
}

TEST_CASE("target mask edge case: MSB is first bit on upper 32 bits", "[generate_target_mask]") {
    uint32_t input[2] = {0x00000001, 0xFFFFFFFF};
    uint32_t target_mask[2];
    uint32_t expected_result[2] = {0x00000000, 0xFFFFFFFF};

    generate_target_mask(input, target_mask);
    ESP_LOGD(TAG, "Result: 0x%08lx%08lx", target_mask[0], target_mask[1]);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_result, target_mask, 2);
}

TEST_CASE("target mask edge case: MSB is in lower 32 bits", "[generate_target_mask]") {
    uint32_t input[2] = {0x00000000, 0xFFFFFFFF};
    uint32_t target_mask[2];
    uint32_t expected_result[2] = {0x00000000, 0x7FFFFFFF};

    generate_target_mask(input, target_mask);
    ESP_LOGD(TAG, "Result: 0x%08lx%08lx", target_mask[0], target_mask[1]);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_result, target_mask, 2);

    input[0] = 0x00000000;
    input[1] = 0x07FFFFFF;
    expected_result[0] = 0x00000000;
    expected_result[1] = 0x03FFFFFF;

    generate_target_mask(input, target_mask);
    ESP_LOGD(TAG, "Result: 0x%08lx%08lx", target_mask[0], target_mask[1]);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_result, target_mask, 2);
}

TEST_CASE("target mask edge case: MSB is first bit in lower 32 bits", "[generate_target_mask]") {
    uint32_t input[2] = {0x00000000, 0x00000001};
    uint32_t target_mask[2];
    uint32_t expected_result[2] = {0x00000000, 0x00000000};

    generate_target_mask(input, target_mask);
    ESP_LOGD(TAG, "Result: 0x%08lx%08lx", target_mask[0], target_mask[1]);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_result, target_mask, 2);
}

TEST_CASE("target mask edge case: 0 diff returns 0 target mask", "[generate_target_mask]") {
    uint32_t input[2] = {0x00000000, 0x00000000};
    uint32_t target_mask[2];
    uint32_t expected_result[2] = {0x00000000, 0x00000000};

    generate_target_mask(input, target_mask);
    ESP_LOGD(TAG, "Result: 0x%08lx%08lx", target_mask[0], target_mask[1]);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_result, target_mask, 2);
}