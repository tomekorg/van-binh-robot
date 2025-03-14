#include "wifi_setup.h"
#include "Arduino.h"
#include "WiFi.h"

static const char *TAG = "WIFI_AP";

void initWifiAp()
{
    const char *ssid = "tomek";
    const char *password = "ptakilatajakluczem";

    IPAddress localIp(192, 168, 1, 22);
    IPAddress gateway(192, 168, 1, 5);
    IPAddress subnet(255, 255, 255, 0);

    ESP_LOGI(TAG, "Configuring AP with static IP...");
    if (WiFi.softAPConfig(localIp, gateway, subnet)) {
        ESP_LOGI(TAG, "AP configuration OK!");
    } else {
        ESP_LOGE(TAG, "Failed to configure AP.");
    }

    ESP_LOGI(TAG, "Starting AP with SSID: %s", ssid);
    if (WiFi.softAP(ssid, password, 1, 0, 4, false, WIFI_AUTH_WPA_WPA2_PSK)) {
        ESP_LOGI(TAG, "AP started OK!");
    } else {
        ESP_LOGE(TAG, "Failed to start AP.");
    }

    IPAddress ip = WiFi.softAPIP();
    ESP_LOGI(TAG, "AP IP address: %s", ip.toString().c_str());
}
