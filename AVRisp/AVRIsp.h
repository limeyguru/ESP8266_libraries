#pragma once
#include <Arduino.h>
#include "command.h"
#include "CommDevice.h"

#define AVRISP_HWVER 2
#define AVRISP_SWMAJ 1
#define AVRISP_SWMIN 18
#define AVRISP_PTIME 10

#define EECHUNK (32)

inline uint16_t beget16(const uint8_t *addr) { return (((uint16_t)addr[0]) << 8 | (uint16_t)addr[1]); }

class AvrISP
{
private:
    // data buffer
    uint8_t buff[256];
    uint8_t buff2[256];
    unsigned int data_index = 0;

    // stk500 parameters
    struct AVRISP_parameter_t
    {
        uint8_t devicecode;
        uint8_t revision;
        uint8_t progtype;
        uint8_t parmode;
        uint8_t polling;
        uint8_t selftimed;
        uint8_t lockbytes;
        uint8_t fusebytes;
        int flashpoll;
        int eeprompoll;
        int pagesize;
        int eepromsize;
        int flashsize;
    };

    // programmer settings, set by remote end
    AVRISP_parameter_t param;

    typedef enum AVR_STATE
    {
        AVR_ERROR = -1,
        AVR_IDLE,
        AVR_WAIT_CMD,
        // AVR_GOT_CMD,
        AVR_GET_DATA,
    } AVR_STATE;

    AVR_STATE avr_state = AVR_IDLE;
    uint8_t avr_cmd = 0;

    int error = 0;
    bool pmode = 0;

    // address for reading and writing, set by 'U' command
    int here;

    WiFiClient _client;

    uint8_t _reset_pin;
    bool _reset_state;
    bool _reset_activehigh;

    inline bool _resetLevel(bool reset_state) { return reset_state == _reset_activehigh; }

#define P_TIME 30

    void pulse(int pin, int times);
    uint8_t getch();
    void respondOK();
    void respondByte(uint8_t data, char response = Resp_STK_OK);
    void respondBuffer(uint8_t *buff, size_t len);
    void respondERR();
    // void breply(uint8_t b, uint8_t type = Resp_STK_OK);
    int getLength();
    void get_parameter(uint8_t c);
    // void fill(int n);
    void set_parameters();
    void start_pmode();
    void end_pmode();
    void setReset(bool rst);
    uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    void write_flash_pages(int length);
    // void write_flash(int length);
    void write_eeprom_chunk(int start, int length);
    void write_eeprom(int length);
    void eeprom_read_page(int length);
    void program_page();
    uint8_t flash_read(uint8_t hilo, int addr);
    void flash_read_page(int length);
    void read_page();
    void universal();
    void read_signature();

public:
    void avrisp();
};
