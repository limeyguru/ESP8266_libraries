#include "CommDevice.h"

#if defined(USING_SERIAL)
SerialDevice comm(BAUD_RATE);
// TelnetDevice debug;
#define debug server
CommDevice::DEVICE_TYPE debug_type = CommDevice::DEVICE_TYPE_TELNET;
#else
SerialDevice debug;
TelnetDevice comm;
// #define comm Serial
CommDevice::DEVICE_TYPE debug_type = CommDevice::DEVICE_TYPE_SERIAL;
#endif

#if true // CommDevice

CommDevice::PORT_DIRECTION CommDevice::direction = CommDevice::PORT_OUTPUT;

bool CommDevice::isDebug()
{
    return (type == debug_type);
}
void CommDevice::logDirection(PORT_DIRECTION dir)
{
    if (direction != dir)
    {
        direction = dir;
        server.forAllConnectedClients([](WiFiClient &connection, CommDevice &comm) { connection.print(comm.direction == PORT_OUTPUT ? "\r\n> " : "\r\n< "); }, *this);
    }
}
void CommDevice::logOutput(const uint8_t *buff, const size_t len)
{
    if (isDebug())
        return;
    logDirection(PORT_OUTPUT);
    data.len = len;
    data.out_buff = buff;
    server.forAllConnectedClients([](WiFiClient &connection, CommDevice &comm) {
        for (size_t i = 0; i < comm.data.len; ++i)
            debug.printf("%02x", (unsigned int)comm.data.out_buff[i]); }, *this);
}

void CommDevice::logOutput(uint8_t c)
{
    if (isDebug())
        return;
    logDirection(PORT_OUTPUT);
    data.ch = c;
    server.forAllConnectedClients([](WiFiClient &connection, CommDevice &comm) {
        for (size_t i = 0; i < comm.data.len; ++i)
            debug.printf("%02x", (unsigned int)comm.data.ch); }, *this);
}
void CommDevice::logOutput(const uint8_t *cp)
{
    if (isDebug())
        return;
    logDirection(PORT_OUTPUT);
    data.out_buff = cp;
    server.forAllConnectedClients([](WiFiClient &connection, CommDevice &comm) {
        const uint8_t *cp = comm.data.out_buff;
        while (*cp)
            debug.printf("%02x", (unsigned int)(*cp++)); }, *this);
}
void CommDevice::logOutput(String st)
{
    if (isDebug())
        return;
    logDirection(PORT_OUTPUT);
    data.str = st;
    server.forAllConnectedClients([](WiFiClient &connection, CommDevice &comm) {
        for (auto c : comm.data.str)
            debug.printf("%02x", (unsigned int)c); }, *this);
}
void CommDevice::logInput(int c)
{
    if (isDebug())
        return;
    logDirection(PORT_INPUT);
    data.ch = c;
    server.forAllConnectedClients([](WiFiClient &connection, CommDevice &comm) {
        for (size_t i = 0; i < comm.data.len; ++i)
            debug.printf("%02x", (unsigned int)comm.data.ch); }, *this);
}

#endif

// SerialDevice

void SerialDevice::initialize()
{
    if (!Serial)
        Serial.begin(baud_rate);
    else
        Serial.updateBaudRate(baud_rate);
}

#if true // TelnetlDevice

void TelnetDevice::initialize()
{
    client = server.Connection();
}

int TelnetDevice::available()
{
    if (client == nullptr)
        client = server.Connection();
    return (client != nullptr && *client) ? client->available() : 0;
}
int TelnetDevice::read()
{
    if (client == nullptr)
        client = server.Connection();
    int c = (client != nullptr && *client) ? client->read() : EOF;
    logInput(c);
    return c;
}
size_t TelnetDevice::write(const uint8_t *buff, size_t len)
{
    if (client == nullptr)
        client = server.Connection();
    logOutput(buff, len);
    return (client != nullptr && *client) ? client->write(buff, len) : 0;
}
size_t TelnetDevice::print(const char data)
{
    if (client == nullptr)
        client = server.Connection();
    logOutput(data);
    return (client != nullptr && *client) ? client->print(data) : 0;
}

size_t TelnetDevice::print(const char *data)
{
    if (client == nullptr)
        client = server.Connection();
    logOutput(data);
    return (client != nullptr && *client) ? client->print(data) : 0;
}

size_t TelnetDevice::println(const char *data)
{
    if (client == nullptr)
        client = server.Connection();
    logOutput(data);
    return (client != nullptr && *client) ? client->println(data) : 0;
}

size_t TelnetDevice::println(const String data)
{
    if (client == nullptr)
        client = server.Connection();
    logOutput(data);
    return (client != nullptr && *client) ? client->println(data) : 0;
}

size_t TelnetDevice::printf(const char *fmt, const uint8_t c)
{
    if (client == nullptr)
        client = server.Connection();
    return (client != nullptr && *client) ? client->printf(fmt, c) : 0;
}

size_t TelnetDevice::printf(const char *fmt, const char *str)
{
    if (client == nullptr)
        client = server.Connection();
    return (client != nullptr && *client) ? client->printf(fmt, str) : 0;
}

void TelnetDevice::stop()
{
    if (client != nullptr && client->connected())
        client->stop();
}

#endif

