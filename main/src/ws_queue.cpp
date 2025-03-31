#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "ws_queue.h"
#include "esp_log.h"

QueueHandle_t wsInQueue = NULL;
QueueHandle_t wsOutQueue = NULL;

void initWsQueues() {
    wsInQueue = xQueueCreate(10, sizeof(WSMessage*));
    if (wsInQueue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create websocket in queue.");
        return;
    }
    
    wsOutQueue = xQueueCreate(10, sizeof(WSMessage*));
    if (wsOutQueue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create websocket out queue.");
        return;
    }
}
