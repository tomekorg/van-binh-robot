#include "Arduino.h"
#include "wifi_setup.h"
#include "http_server.h"
#include "ws_queue.h"
#include "driver/gpio.h"
#include <time.h>
#include <sys/time.h>
#define LEFT_PIN1 GPIO_NUM_18
#define LEFT_PIN2 GPIO_NUM_19
#define RIGHT_PIN1 GPIO_NUM_21
#define RIGHT_PIN2 GPIO_NUM_22

extern "C" void app_main()
{
    initArduino();
    initWsQueues();
    initWifiAp();
    initHttpServer();

    gpio_reset_pin(LEFT_PIN1);
    gpio_reset_pin(LEFT_PIN2);
    gpio_reset_pin(RIGHT_PIN1);
    gpio_reset_pin(RIGHT_PIN2);

    gpio_set_direction(LEFT_PIN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEFT_PIN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(RIGHT_PIN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(RIGHT_PIN2, GPIO_MODE_OUTPUT);

    ledcAttach(LEFT_PIN1, 5000, 8);
    ledcAttach(LEFT_PIN2, 5000, 8);
    ledcAttach(RIGHT_PIN1, 5000, 8);
    ledcAttach(RIGHT_PIN2, 5000, 8);
    
    int left = 0;
    int right = 0;

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

            left = y + x;
            right = y - x;

            delete msg;
        }

        int max_value = max(abs(left), abs(right));
        if (max_value > 1000) {
            left = (left * 1000) / max_value;
            right = (right * 1000) / max_value;
        }
        
        int pwm_left = (left * 255) / 1000;
        int pwm_right = (right * 255) / 1000;

        pwm_left = constrain(pwm_left, -255, 255);
        pwm_right = constrain(pwm_right, -255, 255);

        if (left >= 0) {
            ledcWrite(LEFT_PIN1, pwm_left);
            gpio_set_level(LEFT_PIN2, 0);
        } else {
            ledcWrite(LEFT_PIN2, -pwm_left);
            gpio_set_level(LEFT_PIN1, 0);
        }

        if (right >= 0) {
            ledcWrite(RIGHT_PIN1, pwm_right);
            gpio_set_level(RIGHT_PIN2, 0);
        } else {
            ledcWrite(RIGHT_PIN2, -pwm_right);
            gpio_set_level(RIGHT_PIN1, 0);
        }

        if (left == 0 && right == 0) {
            gpio_set_level(LEFT_PIN1, 0);
            gpio_set_level(LEFT_PIN2, 0);
            gpio_set_level(RIGHT_PIN1, 0);
            gpio_set_level(RIGHT_PIN2, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
