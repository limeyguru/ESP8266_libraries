#include "TelnetServer.h"
#include <WiFiManager.h>

#ifndef BAUD_RATE
#define BAUD_RATE 115200
#endif

void wifiManagerInit()
{
    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager WFM;
    WFM.setDebugOutput(false);

#ifdef TELNET_VERBOSE
    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    WFM.setAPCallback([](WiFiManager *myWiFiManager) {
        Serial.println(F("Entered config mode"));
        Serial.println(WiFi.softAPIP());
        //if you used auto generated SSID, print it
        Serial.println(myWiFiManager->getConfigPortalSSID());
    });
#endif

    // fetches ssid and pass and tries to connect.
    // if it does not connect, it starts an access point with the specified name
    // here  "AutoConnectAP"
    // and goes into a blocking loop awaiting configuration
    if (!WFM.autoConnect())
    {
#ifdef TELNET_VERBOSE
        Serial.println(F("failed to connect and hit timeout"));
#endif
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(1000);
    }

    Serial.println(WiFi.localIP().toString());

    //if you get here you have connected to the WiFi
    Serial.println(F("connected."));
}

void setup()
{
    Serial.begin(BAUD_RATE);
    Serial.println(F("Starting"));
    wifiManagerInit();
    TelnetServer::start();
}

void loop(void)
{
    Telnet.handle();
}
