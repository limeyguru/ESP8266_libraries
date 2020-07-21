#include "TelnetServer.h"

TelnetServer Telnet(TELNET_PORT);

// class TelnetClient methods

bool TelnetServer::AddClient(String pwd, WiFiClient &new_connection)
{
    if (Clients.size() >= MAX_SRV_CLIENTS)
        return false;
    TelnetClient *new_client = new TelnetClient(pwd);
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
        if (client->connected())
            return true;
    }
    return false;
}

Print *TelnetServer::Connection()
{
    for (auto client : Clients)
    {
        if (*client)
            return &(client->_connection);
    }
    return nullptr;
}

bool TelnetServer::ProcessCommand(String& input, Print& connection)
{
#ifdef TELNET_VERBOSE
    connection.println(String(F("Processing command: ")) + input);
#endif
    if (help_processor != nullptr && (*help_processor)(input, connection))
        return true;
    if (cmd_processor != nullptr)
        return (*cmd_processor)(input, connection);
    return false;
}

// Static
void TelnetServer::handle()
{
    int i = 0;
    //check if there are any new clients
    if (this->hasClient())
    {
        WiFiClient connection = available();
        for (auto client : Clients)
        {
            if (client->connected())
                ++i;

            if (i < MAX_SRV_CLIENTS)
            {
                client->connect(connection);
            }
        }
        if (i < MAX_SRV_CLIENTS)
        {
            AddClient(TELNET_PASSWORD, connection);
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
    Telnet.begin();
    Telnet.setNoDelay(true);
#ifdef TELNET_VERBOSE
    String InitMsg1 = F("Telnet server initialized! Port: ");
    String InitMsg2 = F("\nUse a Telnet client to connect.");
    Serial.print(InitMsg1 + String(TELNET_PORT) + InitMsg2);
    // Serial.print(TELNET_PORT, DEC);
    // Serial.println(InitMsg2);
#endif
}

// class TelnetClient methods

// TelnetClient constructor
TelnetClient::TelnetClient(String pwd)
{
    setPassword(pwd);
    inputString.reserve(TELNET_INPUT_BUFFER_LENGTH);
}

// Process client input
void TelnetClient::input()
{
    if (!connected())
        return;

    if (need_prompt)
    {
        _connection.print(Logged ? prompt : "Password: ");
        need_prompt = false;
        inputString.clear();
    }

    while (_connection.available())
    {
        char c = _connection.read();
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
                if (inputString == password)
                {
                    _connection.println("Welcome - aAccess allowed.");
                    Logged = true;
                }
                else
                {
                    if (cntlogin < 3)
                    {
                        cntlogin++;
                        _connection.println("Incorrect password.");
                    }
                    else
                    {
                        _connection.println("Access denied. Connection rejected.");
                        _connection.stop();
                    }
                }
            }
            else
            {
                if (inputString.length() > 0)
                {
                    if (inputString == "logout")
                        _connection.stop();
                    if (!Telnet.ProcessCommand(inputString, _connection))
                        _connection.println(String(F("Not understood: '")) + inputString + "'");
                }
            }
            inputString.clear();
            need_prompt = true;
        }
    }
}

// Connect to an available connection
bool TelnetClient::connect(WiFiClient &new_connection)
{
    if (!connected())
    {
        if (_connection)
            _connection.stop();
        _connection = new_connection;
        Logged = false;
        need_prompt = true;
        cntlogin = 0;

        if (connected())
        {
            _connection.println("ESP8266 NodeMCU V3 Login");
            while (_connection.available())
                _connection.read();
            inputString.clear();
            delay(1);
            if (Logged)
                _connection.println("Access allowed.");
        }
    }
    return connected();
}

// Returns true if the connection is good.
TelnetClient::operator bool()
{
    return _connection;
}
