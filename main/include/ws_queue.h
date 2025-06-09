#ifndef WS_QUEUE_H
#define WS_QUEUE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <ArduinoJson.h>
#include <memory>

// Use JsonDocument which is the new standard in ArduinoJson v7+.
// It will dynamically allocate memory on the heap as needed, which is
// different from the original fixed-size buffer, but is the modern
// way to use the library. The overall memory usage should be comparable.
const size_t JSON_DOCUMENT_SIZE = 512;

struct WSMessage {
  JsonDocument json;
  // The default constructor is sufficient.
};

void initWsQueues();

extern QueueHandle_t wsInQueue;
extern QueueHandle_t wsOutQueue;

#endif