#ifndef WS_QUEUE_H
#define WS_QUEUE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <ArduinoJson.h>

struct WSMessage {
  JsonDocument json;
};

void initWsQueues();

extern QueueHandle_t wsInQueue;
extern QueueHandle_t wsOutQueue;

#endif
