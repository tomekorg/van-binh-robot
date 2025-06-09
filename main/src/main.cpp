#include "Arduino.h"
#include "wifi_setup.h"
#include "http_server.h"
#include "ws_queue.h"
#include "remote_log.h"
#include "robot_control.h"
#include "QTRSensors.h"
#include "esp_log.h"
#include "robot_params.h"
#include <inttypes.h> // Required for PRIu32 macro

// --- Robot Configuration ---
enum RobotMode
{
    MANUAL,
    AUTO,
    CALIBRATING
};

// --- Global Objects & State ---
QTRSensors qtr;
RobotMode currentMode = MANUAL;
bool isCalibrated = false;
const uint8_t SENSOR_COUNT = 8;
const uint8_t sensorPins[] = {23, 22, 21, 19, 18, 17, 16, 4};
uint16_t sensorValues[SENSOR_COUNT];
static uint32_t calibrationStartTime = 0;

// --- FINAL TUNING PARAMETERS ---
float Kp = 0.18f;
float Ki = 0.0005f;
float Kd = 4.5f;
int baseSpeed = 750;
int maxMotorSpeed = 1000;
int minAutoSpeed = 350;
uint32_t ledcFrequency = 49000;

// --- ADVANCED PARAMETERS (NOW TUNABLE) ---
float integralLimit = 3000.0f;
float turnSensitivity = 0.15f; // Note: This parameter is exposed but not used in the current auto logic.
int sharpTurnErrorThreshold = 2000;
float sharpTurnKp = 0.4f;

// PID state variables
static float integral = 0.0f;
static float lastError = 0.0f;

// Arcade drive implementation
void arcadeDrive(int x_turn, int y_drive)
{
    int leftSpeed = y_drive + x_turn;
    int rightSpeed = y_drive - x_turn;
    int maxAbsSpeed = max(abs(leftSpeed), abs(rightSpeed));
    if (maxAbsSpeed > maxMotorSpeed)
    {
        leftSpeed = (leftSpeed * maxMotorSpeed) / maxAbsSpeed;
        rightSpeed = (rightSpeed * maxMotorSpeed) / maxAbsSpeed;
    }
    leftSpeed = constrain(leftSpeed, -maxMotorSpeed, maxMotorSpeed);
    rightSpeed = constrain(rightSpeed, -maxMotorSpeed, maxMotorSpeed);
    setMotorSpeeds(leftSpeed, rightSpeed);
}

extern "C" void app_main()
{
    initArduino();
    initWsQueues();
    initRemoteLog();
    initWifiAp();
    initHttpServer();
    initRobotControl();

    esp_log_level_set("gpio", ESP_LOG_WARN);

    qtr.setTypeRC();
    qtr.setSensorPins(sensorPins, SENSOR_COUNT);
    qtr.setTimeout(2500);
    updateLedcFrequency(ledcFrequency);

    while (true)
    {
        WSMessage *msg = nullptr;
        if (xQueueReceive(wsInQueue, &msg, 0) == pdPASS)
        {
            const char *type = msg->json["type"];
            if (strcmp(type, "joystick") == 0)
            {
                if (currentMode != MANUAL)
                {
                    ESP_LOGI("main", "Joystick used, switching to MANUAL mode");
                    stopMotors();
                    currentMode = MANUAL;
                }
                int x = msg->json["payload"]["x"];
                int y = msg->json["payload"]["y"];
                arcadeDrive(x, y);
            }
            else if (strcmp(type, "start_auto") == 0)
            {
                if (currentMode != AUTO)
                {
                    ESP_LOGI("main", "Switching to AUTO mode");
                    currentMode = AUTO;
                    integral = 0;
                    lastError = 0;
                    stopMotors();
                }
            }
            else if (strcmp(type, "calibrate") == 0)
            {
                if (currentMode != CALIBRATING)
                {
                    ESP_LOGI("main", "Auto-calibration started.");
                    currentMode = CALIBRATING;
                    calibrationStartTime = millis();
                    isCalibrated = false;
                    qtr.resetCalibration();
                }
            }
            else if (strcmp(type, "update_params") == 0)
            {
                JsonObject payload = msg->json["payload"];
                JsonVariant val;
                val = payload["kp"];
                if (!val.isNull()) Kp = val.as<float>();
                val = payload["ki"];
                if (!val.isNull()) Ki = val.as<float>();
                val = payload["kd"];
                if (!val.isNull()) Kd = val.as<float>();
                val = payload["baseSpeed"];
                if (!val.isNull()) baseSpeed = val.as<int>();
                val = payload["maxMotorSpeed"];
                if (!val.isNull()) maxMotorSpeed = val.as<int>();
                val = payload["minAutoSpeed"];
                if (!val.isNull()) minAutoSpeed = val.as<int>();
                val = payload["ledcFrequency"];
                if (!val.isNull())
                {
                    uint32_t new_freq = val.as<uint32_t>();
                    if (new_freq != ledcFrequency)
                    {
                        ledcFrequency = new_freq; // Update global variable
                        updateLedcFrequency(ledcFrequency);
                    }
                }
                // Update new advanced parameters
                val = payload["integralLimit"];
                if (!val.isNull()) integralLimit = val.as<float>();
                val = payload["turnSensitivity"];
                if (!val.isNull()) turnSensitivity = val.as<float>();
                val = payload["sharpTurnErrorThreshold"];
                if (!val.isNull()) sharpTurnErrorThreshold = val.as<int>();
                val = payload["sharpTurnKp"];
                if (!val.isNull()) sharpTurnKp = val.as<float>();

                ESP_LOGI("main", "Params updated: Kp=%.3f, Ki=%.4f, Kd=%.3f, BaseSpeed=%d", Kp, Ki, Kd, baseSpeed);
                ESP_LOGI("main", "Advanced Params: I-Limit=%.1f, SharpThresh=%d, SharpKp=%.2f", integralLimit, sharpTurnErrorThreshold, sharpTurnKp);
            }
            delete msg;
        }

        switch (currentMode)
        {
        case MANUAL:
        {
            static uint32_t lastSendTime = 0;
            if (millis() - lastSendTime > 100)
            {
                lastSendTime = millis();
                if (isCalibrated)
                {
                    qtr.readCalibrated(sensorValues);
                }
                else
                {
                    qtr.read(sensorValues);
                    uint16_t timeout = qtr.getTimeout();
                    for (int i = 0; i < SENSOR_COUNT; i++)
                    {
                        sensorValues[i] = (uint16_t)(((float)sensorValues[i] / timeout) * 1000.0f);
                        sensorValues[i] = constrain(sensorValues[i], 0, 1000);
                    }
                }
                WSMessage *sensorMsg = new WSMessage();
                if (sensorMsg)
                {
                    sensorMsg->json["type"] = "sensors";
                    JsonArray payload = sensorMsg->json["payload"].to<JsonArray>();
                    for (int i = 0; i < SENSOR_COUNT; i++)
                    {
                        payload.add(sensorValues[i]);
                    }
                    if (xQueueSend(wsOutQueue, &sensorMsg, 0) != pdPASS)
                    {
                        ESP_LOGW("main", "Failed to send sensor data to queue");
                        delete sensorMsg;
                    }
                }
            }
            break;
        }
        case AUTO:
        {
            if (!isCalibrated)
            {
                ESP_LOGW("main", "Calibration required. Reverting to MANUAL.");
                stopMotors();
                currentMode = MANUAL;
                break;
            }

            uint16_t position = qtr.readLineBlack(sensorValues);
            const int setpoint = ((SENSOR_COUNT - 1) * 1000) / 2;
            const int max_pos_value = (SENSOR_COUNT - 1) * 1000;

            // --- TIER 3: TOTAL LINE LOSS RECOVERY ---
            if (position == 0 || position == max_pos_value)
            {
                // This is the last resort fail-safe for the sharpest turns or falling off.
                // Execute a powerful pivot turn based on the last known direction.
                if (lastError < 0)
                {
                    setMotorSpeeds(-maxMotorSpeed * 0.8, maxMotorSpeed * 0.8);
                }
                else
                {
                    setMotorSpeeds(maxMotorSpeed * 0.8, -maxMotorSpeed * 0.8);
                }
                integral = 0; // Reset integral when line is lost.
            }
            else
            {
                int error = position - setpoint;

                // --- TIER 2: CONTROLLED SHARP TURN LOGIC ---
                if (abs(error) > sharpTurnErrorThreshold)
                {
                    // For 90-degree turns, use a simpler, more stable P-controller.
                    // This avoids the derivative "kick" that causes stalling.
                    int turn_correction = (int)(sharpTurnKp * error);

                    // Slow to the minimum speed for maximum control and to power through the turn.
                    int turn_speed = minAutoSpeed;

                    int leftSpeed = turn_speed + turn_correction;
                    int rightSpeed = turn_speed - turn_correction;

                    setMotorSpeeds(leftSpeed, rightSpeed);

                    // Reset integral to prevent it from causing issues after the turn.
                    integral = 0;
                }
                else
                {
                    // --- TIER 1: HIGH-SPEED PID FOR STRAIGHTS & GENTLE CURVES ---
                    integral += error;
                    integral = constrain(integral, -integralLimit, integralLimit);

                    float derivative = error - lastError;

                    int correction = (int)(Kp * error + Ki * integral + Kd * derivative);

                    // Use the advanced non-linear speed control for this state.
                    float error_normalized = (float)abs(error) / (float)setpoint;
                    int speed_range = baseSpeed - minAutoSpeed;
                    int speed_drop = (int)(speed_range * (error_normalized * error_normalized));
                    int dynamic_speed = baseSpeed - speed_drop;

                    int leftSpeed = dynamic_speed + correction;
                    int rightSpeed = dynamic_speed - correction;

                    setMotorSpeeds(leftSpeed, rightSpeed);
                }

                // Update lastError at the very end of the cycle for the next derivative calculation.
                lastError = error;
            }
            break;
        }

        case CALIBRATING:
        {
            uint32_t elapsed = millis() - calibrationStartTime;
            const int calibration_speed = 600; // Use a strong, reliable speed for calibration pivots.

            if (elapsed < 8000)
            {
                // CORRECTED: Call setMotorSpeeds directly for a reliable pivot turn.
                if (elapsed < 2000)
                {
                    setMotorSpeeds(-calibration_speed, calibration_speed); // Pivot Left
                }
                else if (elapsed < 6000)
                {
                    setMotorSpeeds(calibration_speed, -calibration_speed); // Pivot Right
                }
                else
                {
                    setMotorSpeeds(-calibration_speed, calibration_speed); // Pivot Left
                }
                qtr.calibrate();
            }
            else
            {
                ESP_LOGI("main", "Calibration finished.");
                stopMotors();
                isCalibrated = true;
                currentMode = MANUAL;
            }
            break;
        }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}