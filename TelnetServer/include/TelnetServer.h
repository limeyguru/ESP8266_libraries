#pragma once
#include <ESP8266WiFi.h>

#ifndef MAX_SRV_CLIENTS
#define MAX_SRV_CLIENTS 1
#endif

#define TELNET_PORT 23

#ifndef TELNET_INPUT_BUFFER_LENGTH
#define TELNET_INPUT_BUFFER_LENGTH 128
#endif

#ifndef TELNET_PASSWORD
#define TELNET_PASSWORD "PassWd0"
#endif

#define CR (0x0d)
#define LF (0x0a)

typedef bool (*CommandProcessor_t)(String&, Print&);

class TelnetClient
{
public:
    TelnetClient(String pwd = "");

    bool connect(WiFiClient& new_client);
    void setPassword(String p) { password = p; Logged = password.length() == 0; }
    void setPrompt(String p) { prompt = p; }
    bool connected() { return _connection && _connection.connected(); }
    WiFiClient connection() { return _connection; }
    operator bool();

private:
    bool Logged = false;
    bool need_prompt = false;
    String prompt = "$";
    String inputString = "";
    byte cntlogin = 0;
    String password;
    WiFiClient _connection;
    void input();

    friend class TelnetServer;
};

class TelnetServer : public WiFiServer
{
public:
    TelnetServer(uint16_t port) : WiFiServer(port) { }

    bool AddClient(String pwd, WiFiClient& new_clien);
    void handle();
    bool hasConnection();
    Print* Connection();
    void onCommand(CommandProcessor_t p) { cmd_processor = p; }
    void onHelp(CommandProcessor_t p) { help_processor = p; }

    static void start();

private:
    std::vector<TelnetClient *> Clients;
    bool (*newclient)(WiFiClient*) = nullptr;
    CommandProcessor_t cmd_processor = nullptr;
    CommandProcessor_t help_processor = nullptr;

    bool ProcessCommand(String& input, Print& connection);

    friend class TelnetClient;
};

extern TelnetServer Telnet;
