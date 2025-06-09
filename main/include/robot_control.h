#ifndef ROBOT_CONTROL_H
#define ROBOT_CONTROL_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include <stdint.h>

// --- Configuration ---
// Adjust these pin assignments to match your robot's wiring to the DRV8833

// Motor A (e.g., Left Motor) - connected to DRV8833 AIN1 and AIN2
#define MOTOR_A_IN1_PIN GPIO_NUM_32 // Connect to DRV8833 AIN1
#define MOTOR_A_IN2_PIN GPIO_NUM_33 // Connect to DRV8833 AIN2

// Motor B (e.g., Right Motor) - connected to DRV8833 BIN1 and BIN2
#define MOTOR_B_IN1_PIN GPIO_NUM_25 // Connect to DRV8833 BIN1
#define MOTOR_B_IN2_PIN GPIO_NUM_26 // Connect to DRV8833 BIN2

// LEDC (PWM) Configuration
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE // ESP32 has 8 high-speed channels

// We need 4 LEDC channels, two for each motor's IN pins
#define LEDC_CHANNEL_A_IN1      LEDC_CHANNEL_0
#define LEDC_CHANNEL_A_IN2      LEDC_CHANNEL_1
#define LEDC_CHANNEL_B_IN1      LEDC_CHANNEL_2
#define LEDC_CHANNEL_B_IN2      LEDC_CHANNEL_3

#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT    // Resolution of PWM duty, e.g., 10-bit (0-1023)

// Calculated maximum duty value based on resolution
#define LEDC_MAX_DUTY           ((1 << LEDC_DUTY_RES) - 1)

// DRV8833 nSLEEP pin - if you want to control it.
// Pulled high on Pololu board, so can be left disconnected if sleep not needed.
// #define MOTOR_NSLEEP_PIN GPIO_NUM_XX // Optional: if you want to control sleep mode

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes GPIOs and LEDC peripheral for DRV8833 motor control.
 *
 * Must be called once before using other motor control functions.
 */
void initRobotControl();

/**
 * @brief Updates the LEDC timer frequency for motor PWM.
 *
 * @param new_frequency The new frequency in Hz.
 */
void updateLedcFrequency(uint32_t new_frequency);

/**
 * @brief Sets the speed and direction for both motors using DRV8833.
 *
 * @param leftSpeed Speed for the left motor. Range: -1000 (full reverse) to 1000 (full forward).
 * 0 means stop (coast).
 * @param rightSpeed Speed for the right motor. Range: -1000 (full reverse) to 1000 (full forward).
 * 0 means stop (coast).
 */
void setMotorSpeeds(int leftSpeed, int rightSpeed);

/**
 * @brief Stops both motors (coast).
 *
 * For DRV8833, this typically means setting both IN pins for each channel LOW.
 * If active braking is desired, this function would need modification.
 */
void stopMotors();

#ifdef __cplusplus
}
#endif

#endif // ROBOT_CONTROL_H