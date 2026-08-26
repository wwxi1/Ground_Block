/**
 * @file    test_Ground_Block.c
 * @brief   Unit tests for Ground_Block.c
 * @note    This file uses Unity test framework for embedded C testing
 */

/* Unity Test Framework */
#define UNITY_INCLUDE_SETUP_STUBS
#include "unity.h"

/* Production code headers */
#include "Ground_Block.h"
#include "DJmotor.h"
#include "motor_config.h"
#include <stdint.h>
#include <stdbool.h>

/* Mock external dependencies */
extern uint8_t solenoid_signal;
extern DJMotor DJmotor[USE_DJNUM];

/* Test fixtures */
static uint8_t original_solenoid_signal;
static bool original_DJmotor_Begin[USE_DJNUM];

/* Setup: Run before each test */
void setUp(void)
{
    /* Save original state */
    original_solenoid_signal = solenoid_signal;

    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        original_DJmotor_Begin[i] = DJmotor[i].Begin;
    }

    /* Reset to initial state before each test */
    solenoid_signal = 0;

    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
        DJmotor[i].ID = i + 1;  /* Set a reasonable ID */
    }
}

/* Teardown: Run after each test */
void tearDown(void)
{
    /* Restore original state */
    solenoid_signal = original_solenoid_signal;

    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = original_DJmotor_Begin[i];
    }
}

/* ========================================================================== */
/* Test Cases for Ground_Block_enable() */
/* ========================================================================== */

/**
 * @test    test_Ground_Block_enable_normal_case
 * @brief   Test that Ground_Block_enable() correctly enables all motors
 * @details Verifies that:
 *          - solenoid_signal is set to 1
 *          - All DJmotor[].Begin fields are set to true (1)
 */
void test_Ground_Block_enable_normal_case(void)
{
    /* Initialize pre-test state */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify solenoid_signal is set to 1 */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);

    /* Verify all motor Begin flags are set to true (1) */
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
    }
}

/**
 * @test    test_Ground_Block_enable_signal_already_set
 * @brief   Test behavior when solenoid_signal is already 1
 * @details Verifies the function still works correctly when called
 *          with pre-existing enabled state
 */
void test_Ground_Block_enable_signal_already_set(void)
{
    /* Initialize pre-test state with signal already set */
    solenoid_signal = 1;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify solenoid_signal remains 1 */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);

    /* Verify all motor Begin flags are set to true */
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
    }
}

/**
 * @test    test_Ground_Block_enable_some_motors_already_enabled
 * @brief   Test behavior when some motors are already enabled
 * @details Verifies that all motors are enabled regardless of
 *          their initial state
 */
void test_Ground_Block_enable_some_motors_already_enabled(void)
{
    /* Initialize pre-test state with mixed states */
    solenoid_signal = 0;
    DJmotor[0].Begin = false;
    DJmotor[1].Begin = true;   /* Already enabled */
    DJmotor[2].Begin = false;
    DJmotor[3].Begin = true;   /* Already enabled */

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify solenoid_signal is set to 1 */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);

    /* Verify all motor Begin flags are set to true */
    TEST_ASSERT_TRUE(DJmotor[0].Begin);
    TEST_ASSERT_TRUE(DJmotor[1].Begin);
    TEST_ASSERT_TRUE(DJmotor[2].Begin);
    TEST_ASSERT_TRUE(DJmotor[3].Begin);
}

/**
 * @test    test_Ground_Block_enable_max_index
 * @brief   Test that the last motor index (USE_DJNUM-1) is processed
 * @details Boundary test for the maximum array index
 */
void test_Ground_Block_enable_max_index(void)
{
    /* Initialize pre-test state */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify solenoid_signal is set to 1 */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);

    /* Specifically verify the last motor index */
    TEST_ASSERT_TRUE(DJmotor[USE_DJNUM - 1].Begin);
}

/**
 * @test    test_Ground_Block_enable_min_index
 * @brief   Test that the first motor index (0) is processed
 * @details Boundary test for the minimum array index
 */
void test_Ground_Block_enable_min_index(void)
{
    /* Initialize pre-test state */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify solenoid_signal is set to 1 */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);

    /* Specifically verify the first motor index */
    TEST_ASSERT_TRUE(DJmotor[0].Begin);
}

/**
 * @test    test_Ground_Block_enable_multiple_calls
 * @brief   Test that the function can be called multiple times
 * @details Verifies idempotency - multiple calls produce same result
 */
void test_Ground_Block_enable_multiple_calls(void)
{
    /* Initialize pre-test state */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
    }

    /* Execute function under test multiple times */
    Ground_Block_enable();

    /* Verify state after first call */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
    }

    /* Call again */
    Ground_Block_enable();

    /* Verify state remains unchanged */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
    }

    /* Third call */
    Ground_Block_enable();

    /* Verify state still remains unchanged */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
    }
}

/**
 * @test    test_Ground_Block_enable_signal_value_type
 * @brief   Test that solenoid_signal is set to the exact uint8_t value 1
 * @details Verifies the exact type and value assignment
 */
void test_Ground_Block_enable_signal_value_type(void)
{
    /* Initialize pre-test state */
    solenoid_signal = 255;  /* Max uint8_t value */

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify exact value assignment */
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, solenoid_signal, "solenoid_signal should be set to exactly 1");
}

/**
 * @test    test_Ground_Block_enable_loop_iteration_count
 * @brief   Test that the loop iterates exactly USE_DJNUM times
 * @details Verifies loop iteration count is correct
 */
void test_Ground_Block_enable_loop_iteration_count(void)
{
    /* Initialize pre-test state */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify exactly USE_DJNUM motors were affected */
    uint32_t enabled_count = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        if (DJmotor[i].Begin)
        {
            enabled_count++;
        }
    }

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(USE_DJNUM, enabled_count, "All motors should be enabled");
}

/**
 * @test    test_Ground_Block_enable_after_disable_state
 * @brief   Test enabling after all motors were previously disabled
 * @details Simulates a disable-then-enable scenario
 */
void test_Ground_Block_enable_after_disable_state(void)
{
    /* Simulate disabled state */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
        DJmotor[i].MODE_Set = DJ_Disable;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify enable state */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
        /* MODE_Set should not be affected by this function */
        TEST_ASSERT_EQUAL_INT(DJ_Disable, DJmotor[i].MODE_Set);
    }
}

/**
 * @test    test_Ground_Block_enable_motor_structure_integrity
 * @brief   Test that other motor structure fields are not modified
 * @details Verifies the function only modifies the Begin field
 */
void test_Ground_Block_enable_motor_structure_integrity(void)
{
    /* Initialize pre-test state with other fields populated */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
        DJmotor[i].ID = i + 1;
        DJmotor[i].MODE_Set = DJ_RPM;
        DJmotor[i].MODE_Cur = DJ_Disable;
        DJmotor[i].valSet.speed_rpm = 1000;
        DJmotor[i].valNow.speed_rpm = 0;
        DJmotor[i].limit.MaxAngle_deg = 180.0f;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify only Begin field was modified */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
        TEST_ASSERT_EQUAL_UINT8(i + 1, DJmotor[i].ID);
        TEST_ASSERT_EQUAL_INT(DJ_RPM, DJmotor[i].MODE_Set);
        TEST_ASSERT_EQUAL_INT(DJ_Disable, DJmotor[i].MODE_Cur);
        TEST_ASSERT_EQUAL_INT16(1000, DJmotor[i].valSet.speed_rpm);
        TEST_ASSERT_EQUAL_INT16(0, DJmotor[i].valNow.speed_rpm);
        TEST_ASSERT_EQUAL_FLOAT(180.0f, DJmotor[i].limit.MaxAngle_deg);
    }
}

/**
 * @test    test_Ground_Block_enable_alternating_states
 * @brief   Test with alternating initial motor states
 * @details Verifies correct behavior with patterned initial states
 */
void test_Ground_Block_enable_alternating_states(void)
{
    /* Initialize alternating pattern */
    solenoid_signal = 0;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = (i % 2 == 0) ? false : true;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify all motors are now enabled */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
    }
}

/**
 * @test    test_Ground_Block_enable_signal_boundary_max
 * @brief   Test behavior when solenoid_signal is at max value
 * @details Boundary test for uint8_t maximum value
 */
void test_Ground_Block_enable_signal_boundary_max(void)
{
    /* Initialize to maximum uint8_t value */
    solenoid_signal = 255;
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        DJmotor[i].Begin = false;
    }

    /* Execute function under test */
    Ground_Block_enable();

    /* Verify signal is set to 1 regardless of previous value */
    TEST_ASSERT_EQUAL_UINT8(1, solenoid_signal);
    for (uint32_t i = 0; i < USE_DJNUM; i++)
    {
        TEST_ASSERT_TRUE(DJmotor[i].Begin);
    }
}

/* ========================================================================== */
/* Main Test Runner */
/* ========================================================================== */

int main(void)
{
    UNITY_BEGIN();

    /* Run all test cases */
    RUN_TEST(test_Ground_Block_enable_normal_case);
    RUN_TEST(test_Ground_Block_enable_signal_already_set);
    RUN_TEST(test_Ground_Block_enable_some_motors_already_enabled);
    RUN_TEST(test_Ground_Block_enable_max_index);
    RUN_TEST(test_Ground_Block_enable_min_index);
    RUN_TEST(test_Ground_Block_enable_multiple_calls);
    RUN_TEST(test_Ground_Block_enable_signal_value_type);
    RUN_TEST(test_Ground_Block_enable_loop_iteration_count);
    RUN_TEST(test_Ground_Block_enable_after_disable_state);
    RUN_TEST(test_Ground_Block_enable_motor_structure_integrity);
    RUN_TEST(test_Ground_Block_enable_alternating_states);
    RUN_TEST(test_Ground_Block_enable_signal_boundary_max);

    return UNITY_END();
}