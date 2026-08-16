#include "unity.h"
#include "stepper_motor.h"
#include "fake_hal.h"
#include "t_velocity.h"
#include <string.h>


/* ============================================================================
 * TEST DEFINITIONS
 * ========================================================================== */

#define TEST_TIM_CHANNEL    1
#define TEST_DIR_PIN        0
#define TEST_STEP_COUNTER   100
#define TEST_STEP_PERIOD    1000

/* ============================================================================
 * TEST GLOBALS
 * ========================================================================== */

static stepper_motor_t motor;
static TIM_HandleTypeDef tim_handle;
static GPIO_TypeDef gpio_port;
static t_velocity test_velocity;

/* ============================================================================
 * VELOCITY TEST HELPERS
 * ========================================================================== */

static t_velocity make_velocity(uint8_t dir, uint16_t step_number, uint32_t period)
{
    return ((period & 0xFFFFU) << 16U) |
           (((uint32_t)step_number & 0x3FFFU) << 2U) |
           ((uint32_t)dir & 0x03U);
}

/* ============================================================================
 * SETUP / TEARDOWN
 * ========================================================================== */

void setUp(void)
{
    memset(&motor, 0, sizeof(motor));
    memset(&tim_handle, 0, sizeof(tim_handle));
    memset(&gpio_port, 0, sizeof(gpio_port));
    test_velocity = 0;
}

void tearDown(void)
{
    // Cleanup po każdym teście
}

/* ============================================================================
 * TEST: INITIALIZATION
 * ========================================================================== */

void test_stepper_motor_init_should_set_all_fields_correctly(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN,
                       TEST_STEP_COUNTER, TEST_STEP_PERIOD);

    TEST_ASSERT_EQUAL_PTR(&tim_handle, motor.tim_handler);
    TEST_ASSERT_EQUAL(TEST_TIM_CHANNEL, motor.tim_channel);
    TEST_ASSERT_EQUAL_PTR(&gpio_port, motor.dir_port);
    TEST_ASSERT_EQUAL(TEST_DIR_PIN, motor.dir_pin);
    TEST_ASSERT_EQUAL(TEST_STEP_COUNTER, motor.step_counter);
    TEST_ASSERT_EQUAL(0, motor.tick_counter);
    TEST_ASSERT_EQUAL(TEST_STEP_PERIOD, motor.step_period);
    TEST_ASSERT_EQUAL(0, motor.pulse);
    TEST_ASSERT_EQUAL(10, motor.homing_step_period);
    TEST_ASSERT_EQUAL(0, motor.is_homing);
}

void test_stepper_motor_init_with_null_motor_should_do_nothing(void)
{
    stepper_motor_init(NULL, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN,
                       TEST_STEP_COUNTER, TEST_STEP_PERIOD);

    TEST_PASS();
}

void test_stepper_motor_init_with_null_timer_should_do_nothing(void)
{
    stepper_motor_init(&motor, NULL, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN,
                       TEST_STEP_COUNTER, TEST_STEP_PERIOD);

    TEST_ASSERT_EQUAL(0, motor.step_counter);
}

void test_stepper_motor_init_with_null_port_should_do_nothing(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       NULL, TEST_DIR_PIN,
                       TEST_STEP_COUNTER, TEST_STEP_PERIOD);

    TEST_ASSERT_EQUAL(0, motor.step_counter);
}

/* ============================================================================
 * TEST: VELOCITY SETTING
 * ========================================================================== */

void test_set_motor_velocity_and_dir_should_set_direction_left(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    test_velocity = make_velocity(DIR_LEFT, 50, 500);
    set_motor_velocity_and_dir(&motor, &test_velocity);

    TEST_ASSERT_EQUAL(500, motor.step_period);
    TEST_ASSERT_EQUAL(50, motor.step_counter);
    TEST_ASSERT_EQUAL(250, motor.pulse);
}

void test_set_motor_velocity_and_dir_should_set_direction_right(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    test_velocity = make_velocity(DIR_RIGHT, 30, 600);
    set_motor_velocity_and_dir(&motor, &test_velocity);

    TEST_ASSERT_EQUAL(600, motor.step_period);
    TEST_ASSERT_EQUAL(30, motor.step_counter);
    TEST_ASSERT_EQUAL(300, motor.pulse);
}

void test_set_motor_velocity_and_dir_with_stop_should_set_pulse_zero(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    test_velocity = make_velocity(DIR_STOP, 50, 500);
    set_motor_velocity_and_dir(&motor, &test_velocity);

    TEST_ASSERT_EQUAL(0, motor.pulse);
}

void test_set_motor_velocity_and_dir_with_null_motor_should_return(void)
{
    test_velocity = make_velocity(DIR_LEFT, 50, 500);
    set_motor_velocity_and_dir(NULL, &test_velocity);
    TEST_PASS();
}

void test_set_motor_velocity_and_dir_with_null_velocity_should_return(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    set_motor_velocity_and_dir(&motor, NULL);
    TEST_PASS();
}

/* ============================================================================
 * TEST: PULSE CHECKING
 * ========================================================================== */

void test_check_next_pulse_when_tick_equals_period_should_generate_pulse(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 5, TEST_STEP_PERIOD);

    motor.pulse = 500;
    motor.tick_counter = TEST_STEP_PERIOD;
    motor.step_counter = 5;

    check_next_pulse(&motor);

    TEST_ASSERT_EQUAL(0, motor.tick_counter);
    TEST_ASSERT_EQUAL(4, motor.step_counter);
}

void test_check_next_pulse_when_tick_less_than_period_should_not_generate_pulse(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 5, TEST_STEP_PERIOD);

    motor.pulse = 500;
    motor.tick_counter = TEST_STEP_PERIOD - 1;
    motor.step_counter = 5;

    check_next_pulse(&motor);

    TEST_ASSERT_EQUAL(TEST_STEP_PERIOD, motor.tick_counter);
    TEST_ASSERT_EQUAL(5, motor.step_counter);
}

void test_check_next_pulse_with_null_motor_should_return(void)
{
    check_next_pulse(NULL);
    TEST_PASS();
}

void test_check_next_pulse_should_wrap_tick_counter_to_zero(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 5, 10);

    motor.pulse = 5;
    motor.tick_counter = 10;

    check_next_pulse(&motor);

    TEST_ASSERT_EQUAL(0, motor.tick_counter);
    TEST_ASSERT_EQUAL(4, motor.step_counter);
}

/* ============================================================================
 * TEST: HOMING
 * ========================================================================== */

void test_start_homing_should_set_homing_mode(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    start_homing(&motor, 1);

    TEST_ASSERT_EQUAL(5, motor.pulse);
    TEST_ASSERT_EQUAL(0, motor.tick_counter);
    TEST_ASSERT_EQUAL(0xFFFF, motor.step_counter);
    TEST_ASSERT_EQUAL(1, motor.is_homing);
}

void test_start_homing_should_set_direction_based_on_parameter(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    start_homing(&motor, 1);
    TEST_ASSERT_EQUAL(1, motor.is_homing);

    stop_homing(&motor);
    start_homing(&motor, 0);
    TEST_ASSERT_EQUAL(1, motor.is_homing);
}

void test_start_homing_with_null_motor_should_return(void)
{
    start_homing(NULL, 1);
    TEST_PASS();
}

void test_stop_homing_should_clear_homing_mode(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    start_homing(&motor, 1);
    stop_homing(&motor);

    TEST_ASSERT_EQUAL(0, motor.pulse);
    TEST_ASSERT_EQUAL(0, motor.tick_counter);
    TEST_ASSERT_EQUAL(0, motor.step_counter);
    TEST_ASSERT_EQUAL(0, motor.is_homing);
}

void test_stop_homing_with_null_motor_should_return(void)
{
    stop_homing(NULL);
    TEST_PASS();
}

/* ============================================================================
 * TEST: INTEGRATION
 * ========================================================================== */

void test_full_cycle_set_velocity_and_check_pulse(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    test_velocity = make_velocity(DIR_LEFT, 3, 10);
    set_motor_velocity_and_dir(&motor, &test_velocity);

    TEST_ASSERT_EQUAL(3, motor.step_counter);
    TEST_ASSERT_EQUAL(10, motor.step_period);

    for (int i = 0; i < 3; i++) {
        motor.tick_counter = 10;
        check_next_pulse(&motor);
    }

    TEST_ASSERT_EQUAL(0, motor.step_counter);
}

void test_homing_cycle_start_and_stop(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    start_homing(&motor, 1);
    TEST_ASSERT_EQUAL(1, motor.is_homing);
    TEST_ASSERT_EQUAL(0xFFFF, motor.step_counter);
    TEST_ASSERT_EQUAL(5, motor.pulse);

    stop_homing(&motor);
    TEST_ASSERT_EQUAL(0, motor.is_homing);
    TEST_ASSERT_EQUAL(0, motor.step_counter);
    TEST_ASSERT_EQUAL(0, motor.pulse);
}

void test_velocity_decoding_with_helpers(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    test_velocity = make_velocity(DIR_LEFT, 1234, 4321);
    set_motor_velocity_and_dir(&motor, &test_velocity);

    TEST_ASSERT_EQUAL(4321, motor.step_period);
    TEST_ASSERT_EQUAL(1234, motor.step_counter);
    TEST_ASSERT_EQUAL(2160, motor.pulse);
}

void test_multiple_velocity_changes(void)
{
    stepper_motor_init(&motor, &tim_handle, TEST_TIM_CHANNEL,
                       &gpio_port, TEST_DIR_PIN, 0, 0);

    // Pierwsza prędkość
    test_velocity = make_velocity(DIR_LEFT, 10, 100);
    set_motor_velocity_and_dir(&motor, &test_velocity);
    TEST_ASSERT_EQUAL(100, motor.step_period);
    TEST_ASSERT_EQUAL(10, motor.step_counter);

    // Zmiana prędkości
    test_velocity = make_velocity(DIR_RIGHT, 20, 200);
    set_motor_velocity_and_dir(&motor, &test_velocity);
    TEST_ASSERT_EQUAL(200, motor.step_period);
    TEST_ASSERT_EQUAL(20, motor.step_counter);
}
