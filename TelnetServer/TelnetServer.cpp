#include <ESP8266WiFi.h>
#include "TelnetServer.h"

TelnetServer server(TELNET_PORT);

// Utility functiom
// Splits a string using specified delimiters & symbols
void SplitString(std::vector<String> &split, const String &string, const String &delimiters, const String &symbols)
{
    int b = 0, e = 0;
    for (auto c : string)
    {
        if (symbols.indexOf(c) >= 0)
        {
            String sub = string.substring(b, e);
            sub.trim();
            if (sub.length())
                split.push_back(sub);
            split.push_back(String(c));
            b = e + 1;
        }
        else if (delimiters.indexOf(c) >= 0)
        {
            String sub = string.substring(b, e);
            sub.trim();
            if (sub.length())
                split.push_back(sub);
            b = e + 1;
        }
        if (++e >= (int)string.length())
        {
            String sub = string.substring(b);
            sub.trim();
            if (sub.length())
                split.push_back(sub);
            break;
        }
    }
}

// class TelnetClient methods

bool TelnetServer::AddClient(String pwd, WiFiClient &new_connection)
{
    if (Clients.size() >= MAX_SRV_CLIENTS)
        return false;
    TelnetClient *new_client = new TelnetClient(pwd, Clients.size());
    if (new_client == nullptr)
        return false;
    new_client->connect(new_connection);
    Clients.push_back(new_client);
    return true;
}

bool TelnetServer::hasConnection()
{
    for (auto client : Clients)
    {
        if (client->Connected())
            return true;
    }
    return false;
}

WiFiClient *TelnetServer::Connection()
{
    for (auto client : Clients)
    {
        if (*client)
            return &(client->connection);
    }
    return nullptr;
}

void TelnetServer::forAllConnectedClients(void (*callback)(WiFiClient &, CommDevice &), CommDevice &comm)
{
    for (auto client : Clients)
    {
        if (client->connection && client->connection.connected())
            callback(client->connection, comm);
    }
}

void TelnetServer::addCommand(COMMAND cmd)
{
    for (auto cmnd : commands)
    {
        if (cmd == cmnd)
            return;
    }
    cmd.name.toUpperCase();
    commands.push_back(cmd);
}

bool TelnetServer::ProcessCommand(String input, WiFiClient &connection)
{
#ifdef VERBOSE
    connection.println(String(F("Processing command: ")) + input);
#endif
    std::vector<String> args;
    input.toUpperCase();
    SplitString(args, input);

    if (args.size() == 0)
        return false; // Ignore empty commands

    for (auto cmd : commands)
    {
        if (args[0] == cmd.name)
            return cmd.proc(args, connection);
    }
    return false;
}

// Static
void TelnetServer::telnet()
{
    int i = 0;
    //check if there are any new clients
    if (this->hasClient())
    {
        WiFiClient connection = available();
        for (auto client : Clients)
        {
            if (client->Connected())
                ++i;

            if (i < MAX_SRV_CLIENTS)
            {
                client->connect(connection);
            }
        }
        if (i < MAX_SRV_CLIENTS)
        {
            AddClient("PassWd0", connection);
        }
        else
        {
            connection.stop();
        }
    }

    for (auto client : Clients)
        client->input();
}

// Static
void TelnetServer::start()
{
    initializeCommands();
    server.begin();
    server.setNoDelay(true);
#ifdef VERBOSE
    String InitMsg1 = F("Telnet server initialized! (Port: ");
    String InitMsg2 = F("\nUse a Telnet client to connect.");
    Serial.print(InitMsg1);
    Serial.print(TELNET_PORT, DEC);
    Serial.println(InitMsg2);
#endif
}

// class TelnetClient methods

// TelnetClient constructor
TelnetClient::TelnetClient(String pwd, int num)
{
    PW = pwd;
    number = num;
    inputString.reserve(TELNET_INPUT_BUFFER_LENGTH);
}

// Process client input
void TelnetClient::input()
{
    if (!Connected())
        return;

    if (!prompt)
    {
        connection.print(Logged ? "$" : "Password: ");
        prompt = true;
        inputString.clear();
    }

    while (connection.available())
    {
        char c = connection.read();
        if (inputString.length() >= TELNET_INPUT_BUFFER_LENGTH)
            inputString.clear();

        if (inputString.isEmpty())
        {
            if (isalnum(c))
                inputString += c;
        }
        else
        {
            if (c == 0x08)
                inputString.remove(inputString.length() - 1);
            if (isgraph(c) || c == 0x20)
                inputString += c;
        }

        if (c == LF)
        {
            if (!Logged)
            {
                if (inputString == PW)
                {
                    connection.println("Access allowed.");
                    Logged = true;
                }
                else
                {
                    if (cntlogin < 3)
                    {
                        cntlogin++;
                        connection.println("Incorrect password.");
                    }
                    else
                    {
                        connection.println("Access denied. Connection rejected.");
                        connection.stop();
                    }
                }
            }
            else
            {
                if (inputString.length() > 0)
                {
                if (!server.ProcessCommand(inputString, connection))
                    connection.println(String(F("Not understood: '")) + inputString + "'");
                }
                else
                    server.ProcessCommand(F("HELP"), connection);
            }
            inputString.clear();
            prompt = false;
        }
    }
}

// #define NO_LOGIN
// Connect to an available connection
bool TelnetClient::connect(WiFiClient &new_connection)
{
    if (!Connected())
    {
        if (connection)
            connection.stop();
        connection = new_connection;
        Logged = false;
        prompt = false;
        cntlogin = 0;

        if (Connected())
        {
            connection.println("ESP8266 NodeMCU V3 Login");
            while (connection.available())
                connection.read();
            inputString.clear();
            delay(1);
#ifdef NO_LOGIN
            // Temp - no login
            Logged = true;
            connection.println("Access allowed.");
#endif
        }
    }
    return Connected();
}

// Returns true if the connection is good.
TelnetClient::operator bool()
{
    return connection;
}
