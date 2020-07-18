#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#define MAX_SRV_CLIENTS 1
#define TELNET_PORT 23
#define TELNET_INPUT_BUFFER_LENGTH 50

#define CR (0x0d)
#define LF (0x0a)

class CommDevice;

typedef bool (*SubProcessor_t)(std::vector<String>&, WiFiClient &);

struct COMMAND
{
    COMMAND(const char* nam, SubProcessor_t p) : name(String(nam)), proc(p) { }
    COMMAND(const COMMAND& cmd) { *this = cmd; }
    String name;
    SubProcessor_t proc;
    bool operator==(const COMMAND& other) { return (other.name == name && other.proc == proc); }
    COMMAND& operator= (const COMMAND& other) { name = other.name; proc = other.proc; return *this; }
};

typedef std::vector<COMMAND> CommandList;

typedef bool (*CommandProcessor_t)(String, WiFiClient&);

class TelnetClient
{
public:
    TelnetClient(String pwd, int num);
    int number;
    bool Logged = false;
    bool prompt = false;
    String inputString = "";
    byte cntlogin = 0;
    String PW;
    WiFiClient connection;

    bool connect(WiFiClient& new_client);
    bool Connected() { return connection && connection.connected(); }
    void input();
    operator bool();
};

class TelnetServer : public WiFiServer
{
public:
    TelnetServer(uint16_t port) : WiFiServer(port) { }

    bool AddClient(String pwd, WiFiClient& new_clien);
    void telnet();
    void clearCommands() { commands.clear(); }
    void addCommand(COMMAND);
    void addCommand(const char* name, SubProcessor_t proc) { COMMAND cmd(name,proc); addCommand(cmd); }
    void addCommands(CommandList& cmds) { for (auto cmd : cmds ) addCommand(cmd); }
    void forAllConnectedClients(void (*)(WiFiClient&, CommDevice&), CommDevice&);
    bool hasConnection();
    WiFiClient* Connection();

    static void start();

private:
    std::vector<TelnetClient *> Clients;
    bool (*newclient)(WiFiClient*) = nullptr;
    CommandList commands;

    bool ProcessCommand(String input, WiFiClient &connection);

    friend class TelnetClient;
};

extern TelnetServer server;

void telnetInit();

void initializeCommands();

void SplitString(std::vector<String> &split, const String &string, const String &delimiters = " ", const String &symbols = "=");
