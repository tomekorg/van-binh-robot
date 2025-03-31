#include "Arduino.h"
#include "wifi_setup.h"
#include "http_server.h"
#include "ws_queue.h"

extern "C" void app_main()
{
    initArduino();
    initWsQueues();
    initWifiAp();
    initHttpServer();

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

            delete msg;
        }
    }
}
