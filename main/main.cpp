#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "WiFi.h"
#include "wifi_setup.h"
#include <WebServer.h>

WebServer server(80);

void handleRoot()
{
    ESP_LOGI("WEBSERVER", "Received GET request for /");
    server.send(200, "text/plain", "hello!");
}

extern "C" void app_main()
{
    initArduino();
    setup_wifi_ap();

    server.on("/", HTTP_GET, handleRoot);
    ESP_LOGI("WEB_SERVER", "GET / handler registered")

    server.begin();
    ESP_LOGI("WEB_SERVER", "Web server started on port 80");

    while (1)
    {
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
