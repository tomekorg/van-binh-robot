#include "robot_control.h"
#include "robot_params.h" // To access the global ledcFrequency for initialization
#include "esp_log.h"
#include "esp_err.h"
#include <cstdlib>    // For abs()
#include <inttypes.h> // For PRIu32 macro

static const char *TAG = "robot_control_drv8833";

// Global ledcFrequency is declared in robot_params.h and defined in main.cpp
// It will be used by initRobotControl.

// Helper function to configure and control a single motor with DRV8833
static void set_single_motor_speed_drv8833(
    gpio_num_t in1_pin, ledc_channel_t in1_channel,
    gpio_num_t in2_pin, ledc_channel_t in2_channel,
    int speed)
{

    // Clamp speed to the operational range [-1000, 1000]
    // This range is used by the PID output; maxMotorSpeed in main.cpp constrains it further.
    if (speed > 1000)
        speed = 1000;
    if (speed < -1000)
        speed = -1000;

    // CRITICAL: Invert speed for one motor if they are physically mounted opposite.
    // This example assumes MOTOR_B (right motor typically) might need inversion.
    // Test your robot: if it spins instead of driving forward, uncomment and adjust.
    // if (in1_pin == MOTOR_B_IN1_PIN) { // Or use a more robust check like motor_is_right_motor
    //     speed = -speed;
    // }
    // Example: If Motor A is Left, Motor B is Right. If Right motor needs inversion:
    if (in1_channel == LEDC_CHANNEL_B_IN1 || in1_channel == LEDC_CHANNEL_B_IN2)
    { // Assuming B is one side
        // speed = -speed; // UNCOMMENT IF YOUR RIGHT MOTOR SPINS THE WRONG WAY
    }

    uint32_t duty = (abs(speed) * LEDC_MAX_DUTY) / 1000;

    if (speed > 0)
    { // Forward: IN1=PWM, IN2=LOW
        ESP_LOGD(TAG, "Motor CH (IN1:%d, IN2:%d) FWD, Speed: %d, Duty: %" PRIu32, in1_pin, in2_pin, speed, duty);
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, in2_channel, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, in2_channel));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, in1_channel, duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, in1_channel));
    }
    else if (speed < 0)
    { // Backward: IN1=LOW, IN2=PWM
        ESP_LOGD(TAG, "Motor CH (IN1:%d, IN2:%d) BCK, Speed: %d, Duty: %" PRIu32, in1_pin, in2_pin, speed, duty);
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, in1_channel, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, in1_channel));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, in2_channel, duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, in2_channel));
    }
    else
    { // Stop (Active Brake): IN1=HIGH, IN2=HIGH for DRV8833 active braking
        ESP_LOGD(TAG, "Motor CH (IN1:%d, IN2:%d) BRAKE, Speed: 0", in1_pin, in2_pin);
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, in1_channel, LEDC_MAX_DUTY));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, in1_channel));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, in2_channel, LEDC_MAX_DUTY));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, in2_channel));
    }
}

void initRobotControl()
{
    ESP_LOGI(TAG, "Initializing Robot Control for DRV8833...");

    // 1. Configure GPIO pins for DRV8833 INx lines.
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTOR_A_IN1_PIN) | (1ULL << MOTOR_A_IN2_PIN) |
                        (1ULL << MOTOR_B_IN1_PIN) | (1ULL << MOTOR_B_IN2_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "Motor INx GPIOs configured.");

    // 2. Configure LEDC timer (common for all channels)
    // Uses the global ledcFrequency from robot_params.h (defined in main.cpp)
    ledc_timer_config_t ledc_timer_cfg = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = ledcFrequency, // Using the global variable
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_cfg));
    ESP_LOGI(TAG, "LEDC Timer configured. Freq: %" PRIu32 " Hz, Res: %d-bit", ledcFrequency, (int)LEDC_DUTY_RES);

    // 3. Configure LEDC channels (one for each INx pin)
    ledc_channel_config_t ledc_channel_conf_template = {
        .gpio_num = 0,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
        .flags = {.output_invert = 0}};

    // Motor A - IN1
    ledc_channel_conf_template.channel = LEDC_CHANNEL_A_IN1;
    ledc_channel_conf_template.gpio_num = MOTOR_A_IN1_PIN;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_conf_template));

    // Motor A - IN2
    ledc_channel_conf_template.channel = LEDC_CHANNEL_A_IN2;
    ledc_channel_conf_template.gpio_num = MOTOR_A_IN2_PIN;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_conf_template));

    // Motor B - IN1
    ledc_channel_conf_template.channel = LEDC_CHANNEL_B_IN1;
    ledc_channel_conf_template.gpio_num = MOTOR_B_IN1_PIN;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_conf_template));

    // Motor B - IN2
    ledc_channel_conf_template.channel = LEDC_CHANNEL_B_IN2;
    ledc_channel_conf_template.gpio_num = MOTOR_B_IN2_PIN;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_conf_template));

    stopMotors(); // Ensure motors are stopped (braked) on initialization

    ESP_LOGI(TAG, "Robot Control for DRV8833 Initialized Successfully.");
}

void updateLedcFrequency(uint32_t new_frequency)
{
    // This function should reconfigure the LEDC timer with the new frequency.
    // The global ledcFrequency in main.cpp should also be updated to reflect this change.
    ESP_LOGI(TAG, "Attempting to update LEDC frequency to %" PRIu32 " Hz", new_frequency);
    if (ledc_set_freq(LEDC_MODE, LEDC_TIMER, new_frequency) == ESP_OK)
    {
        // The global variable `ledcFrequency` (externed from robot_params.h, defined in main.cpp)
        // should be updated by the caller in main.cpp before calling this,
        // or this function needs a way to update it.
        // For now, assume the hardware frequency is updated.
        // The global variable in main.cpp will be updated there.
        ESP_LOGI(TAG, "LEDC hardware frequency updated to %" PRIu32 " Hz by ledc_set_freq.", new_frequency);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to update LEDC frequency via ledc_set_freq.");
    }
}

void setMotorSpeeds(int leftSpeed, int rightSpeed)
{
    ESP_LOGD(TAG, "Setting speeds: Left=%d, Right=%d", leftSpeed, rightSpeed);
    // Assuming MOTOR_A is left, MOTOR_B is right
    set_single_motor_speed_drv8833(MOTOR_A_IN1_PIN, LEDC_CHANNEL_A_IN1,
                                   MOTOR_A_IN2_PIN, LEDC_CHANNEL_A_IN2,
                                   leftSpeed);
    set_single_motor_speed_drv8833(MOTOR_B_IN1_PIN, LEDC_CHANNEL_B_IN1,
                                   MOTOR_B_IN2_PIN, LEDC_CHANNEL_B_IN2,
                                   rightSpeed);
}

void stopMotors()
{
    ESP_LOGI(TAG, "Stopping motors (active brake).");
    setMotorSpeeds(0, 0); // Brake by setting speed to 0 for both motors
}