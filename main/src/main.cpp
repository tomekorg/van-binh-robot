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

    clock_t start_time = clock();

    // Tutaj jest dla Was przygotowana petla, w ktorej ma sie odbywac ruszanie silnikami.
    while (true) {
        WSMessage* msg = nullptr;
        // Ponizsza linijka sprawdza czy dostalismy jakas wiadomosc przez websocket (z telefonu).
        // Jesli tak, to ponizszy if jest "true", a w nim odczytujemy x i y (ktore zalezy od tego jak ruszamy joystickiem).
        if (xQueueReceive(wsInQueue, &msg, pdMS_TO_TICKS(0)) == pdPASS) {
            
            const char* type = msg->json["type"];
            int x = (msg->json)["payload"]["x"];
            int y = (msg->json)["payload"]["y"];
            ESP_LOGI("main", "type: %s, x: %d, y: %d", type, x, y);

            float r = sqrt(x*x + y*y);


            
            // clock_t start_time = clock();
            // gpio_set_level(LED_PIN, 1);
    
            // while(true) {
            //     clock_t now = clock();
            //     double elapsed_ms = (double)(now - start_time) * 1000.0 / CLOCKS_PER_SEC;
                
            //     if (elapsed_ms > 1000 - r) {
            //         gpio_set_level(LED_PIN, 0);
            //         clock_t new_now = clock();

            //         break;
            //     }
            // }

            delete msg;
        }
        int period = 1000;
        clock_t now = clock();
        double elapsed_ms = (double)((now - start_time) * 1000) / CLOCKS_PER_SEC;
        if (elapsed_ms >= period) {
            int pin_state = gpio_get_level(LED_PIN);
            if (pin_state == 0) {
                gpio_set_level(LED_PIN, 1);
            }
            else {
                gpio_set_level(LED_PIN, 0);
            }
            start_time = now;
        }
    }
}
