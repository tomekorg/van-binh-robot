#ifndef WS_QUEUE_H
#define WS_QUEUE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <ArduinoJson.h>
#include <memory>

struct WSMessage {
    JsonDocument json;
    WSMessage() : json(JsonDocument()) { }
};

void initWsQueues();

extern QueueHandle_t wsInQueue;
extern QueueHandle_t wsOutQueue;

#endif
