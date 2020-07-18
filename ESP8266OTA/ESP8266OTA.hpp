#pragma once
// #define NO_GLOBAL_ARDUINOOTA

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>

// typedef std::pair<const char *, const char *> AP_TYPE;
typedef std::vector<WifiAPEntry> AP_LIST;

class ESP8266OTA : public ArduinoOTAClass
{
private:
    ESP8266WiFiMulti wifiMulti;
    AP_LIST aps;
    IPAddress ip_addr;

public:
    ESP8266OTA() : ArduinoOTAClass() {}
    ~ESP8266OTA() {}

    bool AddAPs(AP_LIST &aps, bool replace = true);
    IPAddress Setup(AP_LIST &aps);
    IPAddress Setup();
    void reconnect()
    {
        Serial.println("Reconnecting...");
        while (wifiMulti.run() != WL_CONNECTED)
        { // Wait for the Wi-Fi to connect
            delay(250);
            Serial.print('.');
        }
        Serial.println();
    }
};

extern ESP8266OTA OTA;