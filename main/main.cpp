#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "WiFi.h"
#include "wifi_setup.h"
#include "http_server.h"
#include <WebServer.h>

extern "C" void app_main()
{
    initArduino();
    
    setup_wifi_ap();

    WebServer& server = init_http_server();

    while (1)
    {
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
