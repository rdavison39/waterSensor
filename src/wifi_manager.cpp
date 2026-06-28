#include "wifi_manager.h"
#include "secrets.h"
#include "settings.h"

#include <WiFi.h>

static bool connectToNetwork(const char *ssid, const char *password)
{
    Serial.println();
    Serial.println("================================");
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    Serial.println("================================");

    WiFi.disconnect(true);
    delay(500);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 20000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.println("WiFi Connected");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        setLastWiFiSSID(String(ssid));
        return true;
    }

    Serial.println();
    Serial.print("Failed to connect to ");
    Serial.println(ssid);

    return false;
}

bool connectToWiFi()
{
    String preferred = getLastWiFiSSID();

    if (preferred.length() > 0)
    {
        Serial.print("Preferred WiFi: ");
        Serial.println(preferred);
    }

    // Try the previously successful network first
    if (preferred == WIFI1_SSID)
    {
        if (connectToNetwork(WIFI1_SSID, WIFI1_PASSWORD))
            return true;

        if (connectToNetwork(WIFI2_SSID, WIFI2_PASSWORD))
            return true;
    }
    else if (preferred == WIFI2_SSID)
    {
        if (connectToNetwork(WIFI2_SSID, WIFI2_PASSWORD))
            return true;

        if (connectToNetwork(WIFI1_SSID, WIFI1_PASSWORD))
            return true;
    }
    else
    {
        // First boot (no preferred network yet)
        if (connectToNetwork(WIFI1_SSID, WIFI1_PASSWORD))
            return true;

        if (connectToNetwork(WIFI2_SSID, WIFI2_PASSWORD))
            return true;
    }

    Serial.println();
    Serial.println("********************************");
    Serial.println("Unable to connect to any WiFi");
    Serial.println("Will retry in 30 seconds...");
    Serial.println("********************************");

    return false;
}

void maintainWiFiConnection()
{
    static unsigned long lastRetry = 0;

    if (WiFi.status() == WL_CONNECTED)
        return;

    if (millis() - lastRetry < 30000)
        return;

    lastRetry = millis();

    Serial.println();
    Serial.println("[WiFi] Connection lost");

    connectToWiFi();
}