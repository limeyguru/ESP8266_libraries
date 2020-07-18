#include "ESP8266OTA.hpp"

#define LOGON_TIMEOUT (30 * 1000000)

const char *const err_names[] = {
    "Auth Failed",
    "Begin Failed",
    "Connect Failed",
    "Receive Failed",
    "End Failed"};

bool ESP8266OTA::AddAPs(AP_LIST &aps, bool replace)
{
    if (replace)
        wifiMulti.cleanAPlist();
    if (aps.empty() && Serial)
        Serial.println("ESP8266OTA: Setup with empty AP list");
    bool result = true;
    for (auto ap : aps)
        result = result && wifiMulti.addAP(ap.ssid, ap.passphrase);
    return result;
}

IPAddress ESP8266OTA::Setup()
{
    AP_LIST ap_list;
    return Setup(ap_list);
}

IPAddress ESP8266OTA::Setup(AP_LIST &aps)
{
    this->aps = aps;
    AddAPs(aps);

    Serial.println("Connecting ...");
    unsigned long start_time = millis();
    bool failed_logon = false;
    wl_status_t status = WL_IDLE_STATUS;
    // WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& ev)
    // {
    //     Serial.println(String("WiFi disconnected from ") + ev.ssid);
    //     OTA.reconnect();
    // });
    WiFi.setAutoReconnect(true);
    while ((status = wifiMulti.run()) != WL_CONNECTED)
    { // Wait for the Wi-Fi to connect
        if (millis() > (start_time + LOGON_TIMEOUT))
        {
            failed_logon = true;
            break;
        }
        delay(250);
        Serial.print('.');
    }
    if (failed_logon)
    {
        Serial.println("Could not connect to WiFi.");
        Serial.println(String("Last error: ") + err_names[status]);
        Serial.println("Found APs:");
        int count = 0;
        for (auto ap : aps)
        {
            if (wifiMulti.existsAP(ap.ssid))
            {
                Serial.println(ap.ssid);
                ++count;
            }
        }
        if (!count)
            Serial.println("None.");
    }
    else
    {
        Serial.println();
        Serial.print("Connected to\t");
        Serial.println(WiFi.SSID()); // Tell us what network we're connected to
        Serial.print("IP address:\t");
        ip_addr = WiFi.localIP();
        Serial.println(ip_addr); // Send the IP address of the ESP8266 to the computer
    }
    OTA.setHostname("ESP8266");
    OTA.setPassword("esp8266");

    OTA.onStart([]() { Serial.println("Start"); });
    OTA.onEnd([]() { Serial.println("\nEnd"); });
    OTA.onProgress([](unsigned int progress, unsigned int total) { Serial.printf("Progress: %u%% \r", (progress / (total / 100))); });
    OTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error >= 0 && error < 5)
            Serial.println(err_names[error]);
        else
            Serial.printf("Unknown error: %d", error);
    });
    OTA.begin();
    Serial.println("OTA ready");
    return ip_addr;
}

ESP8266OTA OTA;