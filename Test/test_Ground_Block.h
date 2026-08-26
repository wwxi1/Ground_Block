/**
 * @file    test_Ground_Block.h
 * @brief   Unit test declarations for Ground_Block.c
 * @details This header provides declarations for test-specific
 *          mock functions and test fixtures
 */

#ifndef TEST_GROUND_BLOCK_H
#define TEST_GROUND_BLOCK_H

#include <stdint.h>
#include <stdbool.h>

/* Test configuration */
#define TEST_DJMOTOR_COUNT 4U  /* Should match USE_DJNUM from motor_config.h */

/* Test helper function declarations */
void setup_test_environment(void);
void teardown_test_environment(void);
void reset_motor_states(void);
void reset_signal_state(void);

/* Test data structures for validation */
typedef struct
{
    uint32_t test_count;
    uint32_t passed_count;
    uint32_t failed_count;
} test_statistics_t;

/* Global test statistics */
extern test_statistics_t g_test_stats;

/* Helper macros */
#define TEST_CHECK_ALL_MOTORS_ENABLED(expected_value) \
    do { \
        for (uint32_t i = 0; i < USE_DJNUM; i++) { \
            TEST_ASSERT_EQUAL_MESSAGE(expected_value, DJmotor[i].Begin, \
                "Motor Begin flag should match expected value"); \
        } \
    } while(0)

#define TEST_CHECK_SIGNAL(expected_value) \
    TEST_ASSERT_EQUAL_MESSAGE(expected_value, solenoid_signal, \
        "solenoid_signal should match expected value")

#endif /* TEST_GROUND_BLOCK_H */