#ifndef ROBOT_PARAMS_H
#define ROBOT_PARAMS_H

#include <stdint.h>

// Make robot parameters externally accessible so other components
// can read their current values.
extern float Kp;
extern float Ki;
extern float Kd;
extern int baseSpeed;
extern int maxMotorSpeed;
extern int minAutoSpeed;
extern uint32_t ledcFrequency;

// New parameters for PID enhancements, now tunable from the UI
extern float integralLimit;
extern float turnSensitivity;
extern int sharpTurnErrorThreshold;
extern float sharpTurnKp;

#endif // ROBOT_PARAMS_H