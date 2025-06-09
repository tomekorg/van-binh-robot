#include "http_server.h"
#include <ESPAsyncWebServer.h>
#include "ws_queue.h"
#include "bundle_html.h"
#include <ArduinoJson.h>
#include "freertos/task.h" // Required for creating tasks
#include "robot_params.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static const char *TAG = "HTTP/WS SERVER";

// Task to process the outgoing WebSocket message queue
void ws_sender_task(void *pvParameters) {
    WSMessage *msg;
    while (true) {
        // Wait indefinitely for a message to arrive in the queue
        if (xQueueReceive(wsOutQueue, &msg, portMAX_DELAY) == pdPASS) {
            if (msg != nullptr) {
                // Serialize the JSON message to a string
                std::string jsonString;
                serializeJson(msg->json, jsonString);
                
                // Send the string to all connected WebSocket clients
                ws.textAll(jsonString.c_str());
                
                // Free the memory allocated for the message
                delete msg;
            }
        }
    }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
    {
        ESP_LOGI(TAG, "WebSocket client #%lu connected from %s", client->id(),
                 client->remoteIP().toString().c_str());

        // On connection, send the current PID values to the client.
        WSMessage *pid_msg = new WSMessage();
        if (pid_msg)
        {
            pid_msg->json["type"] = "pid_state";
            // Use the new ArduinoJson v7 API to create a nested object.
            JsonObject payload = pid_msg->json["payload"].to<JsonObject>();
            payload["kp"] = Kp;
            payload["ki"] = Ki;
            payload["kd"] = Kd;
            payload["baseSpeed"] = baseSpeed;
            payload["maxMotorSpeed"] = maxMotorSpeed;
            payload["minAutoSpeed"] = minAutoSpeed;
            payload["ledcFrequency"] = ledcFrequency;
            
            // Add new tunable parameters
            payload["integralLimit"] = integralLimit;
            payload["turnSensitivity"] = turnSensitivity;
            payload["sharpTurnErrorThreshold"] = sharpTurnErrorThreshold;
            payload["sharpTurnKp"] = sharpTurnKp;

            if (xQueueSend(wsOutQueue, &pid_msg, pdMS_TO_TICKS(100)) != pdPASS)
            {
                ESP_LOGW("ws_queue", "Failed to send PID state to queue");
                delete pid_msg;
            }
        }
        break;
    }
    case WS_EVT_DISCONNECT:
        ESP_LOGI(TAG, "WebSocket client #%lu disconnected", client->id());
        break;
    case WS_EVT_DATA:
        ESP_LOGD(TAG, "WebSocket data received: %u bytes", len);
        if (len > 0)
        {
            WSMessage *msg = new WSMessage();
            DeserializationError error = deserializeJson(msg->json, data, len);
            if (error)
            {
                ESP_LOGE(TAG, "Failed to parse JSON: %s", error.c_str());
                delete msg;
            }
            else
            {
                if (xQueueSend(wsInQueue, &msg, pdMS_TO_TICKS(100)) != pdPASS)
                {
                    ESP_LOGW("ws_queue", "Failed to send message to the queue");
                    delete msg; // Prevent memory leak if send fails
                }
            }
        }
        break;
    default:
        break;
    }
}

void handleRoot(AsyncWebServerRequest *request)
{
    ESP_LOGI(TAG, "Received GET request on /");
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", (const char *)bundle_html);

    // Ask browser to revalidate to make sure they get latest web-ui
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");

    request->send(response);
}


// Uruchamia serwer HTTP oraz serwer WebSocket.
void initHttpServer()
{
    server.on("/", HTTP_GET, handleRoot);
    ESP_LOGI(TAG, "GET / handler registered");

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.begin();
    ESP_LOGI(TAG, "HTTP server started on port 80");

    // Create and start the WebSocket sender task
    xTaskCreate(ws_sender_task, "ws_sender", 4096, NULL, 5, NULL);
}