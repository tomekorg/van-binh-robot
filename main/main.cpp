#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "WiFi.h"
#include "wifi_setup.h"

extern "C" void app_main()
{
    initArduino();
    setup_wifi_ap();
}
