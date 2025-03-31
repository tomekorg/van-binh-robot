#include "http_server.h"
#include <ESPAsyncWebServer.h>
#include "ws_queue.h"
#include "bundle_html.h"
#include <ArduinoJson.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static const char *TAG = "HTTP/WS SERVER";

// Ta funkcja robi kilka rzeczy:
// 1. Odbiera requesty przez websocket, np. gdy ruszamy joystickiem.
// 2. Loguje informacje do terminala za pomoca ESP_LOGI itd.
// 3. Przetwarza odebrane dane: zamienia je z tekstu na obiekt JSON.
// 4. Wysyła przetworzone dane do kolejki, aby mogły być dalej użyte w main.cpp.
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    ESP_LOGI(TAG, "WebSocket client #%lu connected from %s", client->id(),
             client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    ESP_LOGI(TAG, "WebSocket client #%lu disconnected", client->id());
    break;
  case WS_EVT_DATA:
    ESP_LOGD(TAG, "WebSocket data received: %u bytes", len);
    if (len > 0)
    {
      WSMessage* msg = new WSMessage();
      DeserializationError error = deserializeJson(msg->json, data, len);
      if (error)
      {
        ESP_LOGE(TAG, "Failed to parse JSON: %s", error.c_str());
      }
      else
      {
        if (xQueueSend(wsInQueue, &msg, pdMS_TO_TICKS(100)) != pdPASS)
        {
          ESP_LOGW("ws_queue", "Failed to send message to the queue");
          // Handle the error: possibly retry or log the occurrence.
        }
      }
    }
    break;
  default:
    break;
  }
}

// Obsługuje requesty HTTP GET skierowane do "/".
// Narazie odpowiada na kazdy request tekstem "hello!".
// Docelowo bedzie wysylac strone HTML.
void handleRoot(AsyncWebServerRequest *request)
{
  ESP_LOGI("WEB_SERVER", "Received GET request on /");
  request->send(200, "text/html", (const char *)bundle_html);
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
}