#include "Arduino.h"
#include "wifi_setup.h"
#include "http_server.h"
#include "ws_queue.h"
#include "driver/gpio.h"
#include <time.h>
#include <sys/time.h>
#define LED_PIN GPIO_NUM_2

void blink()
{
    int period = 1000;
}

extern "C" void app_main()
{
    initArduino();
    initWsQueues();
    initWifiAp();
    initHttpServer();

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    int64_t start_time = esp_timer_get_time();
    bool led_on = false;
    float r = 0;

    // Tutaj jest dla Was przygotowana petla, w ktorej ma sie odbywac ruszanie silnikami.
    while (true)
    {
        WSMessage *msg = nullptr;
        // Ponizsza linijka sprawdza czy dostalismy jakas wiadomosc przez websocket (z telefonu).
        // Jesli tak, to ponizszy if jest "true", a w nim odczytujemy x i y (ktore zalezy od tego jak ruszamy joystickiem).
        if (xQueueReceive(wsInQueue, &msg, pdMS_TO_TICKS(0)) == pdPASS)
        {

            const char *type = msg->json["type"];
            int x = (msg->json)["payload"]["x"];
            int y = (msg->json)["payload"]["y"];
            ESP_LOGI("main", "type: %s, x: %d, y: %d", type, x, y);

            r = x * x + y * y;

            delete msg;
        }

        int64_t now = esp_timer_get_time();
        if (now - start_time >= 1000500 - r)
        {
            if (led_on)
            {
                gpio_set_level(LED_PIN, 0);
                led_on = false;
            }
            else
            {
                gpio_set_level(LED_PIN, 1);
                led_on = true;
            }
            start_time = now;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
