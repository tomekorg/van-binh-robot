#include "wifi_setup.h"
#include "Arduino.h"
#include "WiFi.h"

void setup_wifi_ap()
{
    const char *ssid = "tomek";
    const char *password = "ptakilatajakluczem";

    IPAddress local_IP(192, 168, 1, 22);
    IPAddress gateway(192, 168, 1, 5);
    IPAddress subnet(255, 255, 255, 0);

    ESP_LOGI("AP_SETUP", "Setting up Access Point ... ");
    bool is_ap_ok = WiFi.softAPConfig(local_IP, gateway, subnet);
    if (is_ap_ok == true)
    {
        ESP_LOGI("AP_SETUP", "AP is OK!");
    }
    else
    {
        ESP_LOGE("AP_SETUP", "AP is shit!");
    }

    ESP_LOGI("AP_SETUP", "Starting Access Point ... ");
    bool is_ap_working = WiFi.softAP(ssid, password, 1, 0, 4, false, WIFI_AUTH_WPA_WPA2_PSK);
    if (is_ap_working == true)
    {
        ESP_LOGI("AP_SETUP", "AP is working!");
    }
    else
    {
        ESP_LOGE("AP_SETUP", "AP is not working!");
    }

    IPAddress ip = WiFi.softAPIP();

    ESP_LOGI("AP_SETUP", "IP address: %s", ip.toString().c_str());
}
