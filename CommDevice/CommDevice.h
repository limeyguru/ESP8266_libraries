#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "TelnetServer.h"

#define USING_SERIAL
#define BAUD_RATE 115200

struct CallbackData
{
    const uint8_t *in_buff;
    const uint8_t *out_buff;
    uint8_t ch;
    size_t len = 0;
    String str = "";
};

class CommDevice
{
public:
    typedef enum DEVICE_TYPE
    {
        DEVICE_TYPE_SERIAL,
        DEVICE_TYPE_TELNET
    } DEVICE_TYPE;

    typedef enum PORT_DIRECTION
    {
        PORT_INPUT,
        PORT_OUTPUT
    } PORT_DIRECTION;

    CommDevice(DEVICE_TYPE device_type) : type(device_type) {}
    const DEVICE_TYPE type;
    CallbackData data;
    virtual void initialize() = 0;
    virtual int available() = 0;
    virtual int read() = 0;
    virtual size_t write(const uint8_t *buff, size_t len) = 0;
    virtual size_t print(const char data) = 0;
    virtual size_t print(const char *data) = 0;
    virtual size_t println(const char *data) = 0;
    virtual size_t println(const String data) = 0;
    virtual size_t printf(const char *fmt, const uint8_t c) = 0;
    virtual size_t printf(const char *fmt, const char *c) = 0;
    virtual void stop() = 0;
    bool isDebug();
    void logOutput(const uint8_t *buff, const size_t len);
    static void debugOutput(const uint8_t *buff, const size_t len);
    void logOutput(const uint8_t c);
    void logOutput(const uint8_t *c);
    void logOutput(const String c);
    // void logOutput(const char * fmt, int c);
    void logInput(const int c);
    void logDirection(PORT_DIRECTION dir);

private:
    static PORT_DIRECTION direction;
};

class SerialDevice : public CommDevice
{
public:
    SerialDevice(unsigned long baud) : CommDevice(DEVICE_TYPE_SERIAL) { baud_rate = baud; }
    void initialize() override;
    int available() override { return Serial.available(); }
    int read() override
    {
        int c = Serial.read();
        logInput(c);
        return c;
    }
    size_t write(const uint8_t *buff, size_t len) override
    {
        logOutput(buff, len);
        return Serial.write(buff, len);
    }
    size_t print(char data) override
    {
        logOutput(data);
        return Serial.print(data);
    }
    size_t print(const char *data)
    {
        logOutput(data);
        return Serial.print(data);
    }
    size_t println(const char *data) override
    {
        logOutput(data);
        return Serial.println(data);
    }
    size_t println(String data) override
    {
        logOutput(data);
        return Serial.println(data);
    }
    size_t printf(const char *fmt, const uint8_t c) override { return Serial.printf(fmt, c); }
    size_t printf(const char *fmt, const char *c) override { return Serial.printf(fmt, c); }
    void stop() override {}

private:
    unsigned long baud_rate;
};

class TelnetDevice : public CommDevice
{
public:
    TelnetDevice() : CommDevice(DEVICE_TYPE_TELNET) {}
    void initialize() override;
    int available() override;
    int read() override;
    size_t write(const uint8_t *buff, size_t len) override;
    size_t print(const char *data) override;
    size_t print(const char data) override;
    size_t println(const char *data) override;
    size_t println(const String data) override;
    size_t printf(const char *fmt, const uint8_t c) override;
    size_t printf(const char *fmt, const char *c) override;
    void stop() override;

private:
    WiFiClient *client = nullptr;
};

#if defined(USING_SERIAL)
extern SerialDevice comm;
// extern TelnetDevice debug;
#define debug server
#else
extern SerialDevice debug;
extern TelnetDevice comm;
// #define comm Serial
#endif
