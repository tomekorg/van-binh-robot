#include "Arduino.h"
#include "wifi_setup.h"
#include "http_server.h"

extern "C" void app_main()
{
    initArduino();
    setup_wifi_ap();
    init_http_server();
}
