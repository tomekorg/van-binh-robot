#include "Arduino.h"
#include "wifi_setup.h"
#include "http_server.h"
#include "ws_queue.h"
#include "driver/gpio.h"
#define LED_PIN GPIO_NUM_2

extern "C" void app_main()
{
    initArduino();
    initWsQueues();
    initWifiAp();
    initHttpServer();

    // gpio_reset_pin(LED_PIN);
    // gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // while(true) {
    //     gpio_set_level(LED_PIN, 1);
    //     vTaskDelay(pdMS_TO_TICKS(500));

    //     gpio_set_level(LED_PIN, 0);
    //     vTaskDelay(pdMS_TO_TICKS(500));
    // }

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Tutaj jest dla Was przygotowana petla, w ktorej ma sie odbywac ruszanie silnikami.
    while (true) {
        WSMessage* msg = nullptr;
        // Ponizsza linijka sprawdza czy dostalismy jakas wiadomosc przez websocket (z telefonu).
        // Jesli tak, to ponizszy if jest "true", a w nim odczytujemy x i y (ktore zalezy od tego jak ruszamy joystickiem).
        if (xQueueReceive(wsInQueue, &msg, pdMS_TO_TICKS(100)) == pdPASS) {
            const char* type = msg->json["type"];
            int x = (msg->json)["payload"]["x"];
            int y = (msg->json)["payload"]["y"];
            ESP_LOGI("main", "type: %s, x: %d, y: %d", type, x, y);

            if (x>0) {
                gpio_set_level(LED_PIN, 1);
            }


            delete msg;
        }
        else gpio_set_level(LED_PIN, 0);
    }
}
