#include "http_server.h"
#include "Arduino.h"
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static const char *TAG = "HTTP/WS SERVER";

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
  case WS_EVT_CONNECT:
    ESP_LOGI(TAG, "WebSocket client #%lu connected from %s", client->id(),
             client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    ESP_LOGI(TAG, "WebSocket client #%lu disconnected", client->id());
    break;
  case WS_EVT_DATA:
    ESP_LOGI(TAG, "WebSocket data received: %u bytes", len);
    break;
  default:
    break;
  }
}

void handleRoot(AsyncWebServerRequest *request) {
  ESP_LOGI("WEB_SERVER", "Received GET request on /");
  request->send(200, "text/plain", "hello!");
}

void init_http_server() {
  server.on("/", HTTP_GET, handleRoot);
  ESP_LOGI(TAG, "GET / handler registered");

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();
  ESP_LOGI(TAG, "HTTP server started on port 80");
}