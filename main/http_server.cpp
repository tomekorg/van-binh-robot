#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "WiFi.h"
#include <WebServer.h>
#include "http_server.h"

WebServer server(80);

void handleRoot()
{
    ESP_LOGI("WEBSERVER", "Received GET request for /");
    server.send(200, "text/plain", "hello!");
}

WebServer& init_http_server()
{
    server.on("/", HTTP_GET, handleRoot);
    ESP_LOGI("WEB_SERVER", "GET / handler registered");

    server.begin();
    ESP_LOGI("WEB_SERVER", "Web server started on port 80");

    return server;
}