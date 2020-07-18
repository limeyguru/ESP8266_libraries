#include "AVRIsp.h"

void AvrISP::pulse(int pin, int times)
{
    do
    {
        digitalWrite(pin, HIGH);
        delay(P_TIME);
        digitalWrite(pin, LOW);
        delay(P_TIME);
    } while (times--);
}

uint8_t getch()
{
    while (!comm.available())
        yield();
    return comm.read();
}

void AvrISP::respondOK()
{
    comm.print((char)Resp_STK_INSYNC);
    comm.print((char)Resp_STK_OK);
    debug.print("respond OK\r\n");
}

void AvrISP::respondByte(uint8_t data, char response)
{
    comm.print((char)Resp_STK_INSYNC);
    comm.print((char)data);
    comm.print(response);
    debug.print("respond Resp_STK_OK with 1 byte data\r\n");
}

void AvrISP::respondBuffer(uint8_t *buff, size_t len)
{
    comm.print((char)Resp_STK_INSYNC);
    comm.write(buff, len);
    comm.print((char)Resp_STK_OK);
    debug.printf("respond Resp_STK_OK with %d bytes data\r\n", len);
}

void AvrISP::respondERR()
{
    comm.print((char)Resp_STK_NOSYNC);
    error++;
    debug.print("respond Resp_STK_NOSYNC\r\n");
}

// void breply(uint8_t b, uint8_t type = Resp_STK_OK)
// {
//     if (Sync_CRC_EOP == getch())
//     {
//         uint8_t resp[3];
//         resp[0] = Resp_STK_INSYNC;
//         resp[1] = b;
//         resp[2] = type;
//         comm.write((const uint8_t *)resp, (size_t)3);
//     }
//     else
//     {
//         respondERR();
//     }
// }

int AvrISP::getLength()
{
    return 256 * buff[0] + buff[1];
}

void AvrISP::get_parameter(uint8_t c)
{
    switch (c)
    {
    case 0x80:
        respondByte(AVRISP_HWVER);
        break;
    case 0x81:
        respondByte(AVRISP_SWMAJ);
        break;
    case 0x82:
        respondByte(AVRISP_SWMIN);
        break;
    case 0x93:
        respondByte('S'); // serial programmer
        break;
    default:
        respondByte(c, Resp_STK_FAILED);
    }
}

// void fill(int n)
// {
//     // AVRISP_DEBUG("fill(%u)", n);
//     for (int x = 0; x < n; x++)
//     {
//         buff[x] = getch();
//     }
// }

void AvrISP::set_parameters()
{
    // call this after reading paramter packet into buff[]
    param.devicecode = buff[0];
    param.revision = buff[1];
    param.progtype = buff[2];
    param.parmode = buff[3];
    param.polling = buff[4];
    param.selftimed = buff[5];
    param.lockbytes = buff[6];
    param.fusebytes = buff[7];
    param.flashpoll = buff[8];
    // ignore buff[9] (= buff[8])
    // following are 16 bits (big endian)
    param.eeprompoll = beget16(&buff[10]);
    param.pagesize = beget16(&buff[12]);
    param.eepromsize = beget16(&buff[14]);

    // 32 bits flashsize (big endian)
    param.flashsize = buff[16] * 0x01000000   // << 24
                      + buff[17] * 0x00010000 // << 16
                      + buff[18] * 0x00000100 // <<  8
                      + buff[19];
}

void AvrISP::start_pmode()
{
    // SPI.begin();
    // SPI.setFrequency(_spi_freq);
    // SPI.setHwCs(false);

    // // try to sync the bus
    // SPI.transfer(0x00);

    digitalWrite(_reset_pin, _resetLevel(false));
    delayMicroseconds(50);
    digitalWrite(_reset_pin, _resetLevel(true));
    delay(30);

    // spi_transaction(0xAC, 0x53, 0x00, 0x00);
    pmode = 1;
}

void AvrISP::setReset(bool rst)
{
    _reset_state = rst;
    digitalWrite(_reset_pin, _resetLevel(_reset_state));
}

void AvrISP::end_pmode()
{
    // SPI.end();
    setReset(_reset_state);
    pmode = 0;
}

uint8_t AvrISP::spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    // SPI.transfer(a);
    // SPI.transfer(b);
    // SPI.transfer(c);
    // return SPI.transfer(d);
    return 0;
}

void AvrISP::write_flash_pages(int length)
{
    // int x = 0;
    // int page = addr_page(here);
    // while (x < length) {
    //     yield();
    //     if (page != addr_page(here)) {
    //         commit(page);
    //         page = addr_page(here);
    //     }
    //     flash(LOW, here, buff[x++]);
    //     flash(HIGH, here, buff[x++]);
    //     here++;
    // }
    // commit(page);
    // return Resp_STK_OK;
}

// void write_flash(int length)
// {
//     if (data_index == length)
//     {
//         write_flash_pages(length));
//         respondOK();
//     }
//     else
//     {
//         respondERR();
//     }
// }

// write (length) bytes, (start) is a byte address
void AvrISP::write_eeprom_chunk(int start, int length)
{
    // this writes byte-by-byte,
    // page writing may be faster (4 bytes at a time)
    // fill(length);
    // prog_lamp(LOW);
    // for (int x = 0; x < length; x++)
    // {
    //     // int addr = start + x;
    //     // spi_transaction(0xC0, (addr >> 8) & 0xFF, addr & 0xFF, buff[x]);
    //     delay(45);
    // }
    // prog_lamp(HIGH);
    // return Resp_STK_OK;
}

void AvrISP::write_eeprom(int length)
{
    // here is a word address, get the byte address
    int start = here * 2;
    int remaining = length;
    while (remaining > EECHUNK)
    {
        write_eeprom_chunk(start, EECHUNK);
        start += EECHUNK;
        remaining -= EECHUNK;
    }
    if (remaining)
        write_eeprom_chunk(start, remaining);
}

void AvrISP::program_page()
{
    if (data_index < 3)
    {
        respondERR();
        return;
    }
    int length = getLength();
    char memtype = buff[2];
    if ((int)data_index != (length + 3))
    {
        respondERR();
        return;
    }
    // flash memory @here, (length) bytes
    if (memtype == 'F')
    {
        write_flash_pages(length);
        respondOK();
        return;
    }

    if (memtype == 'E')
    {
        if (length > param.eepromsize)
        {
            respondERR();
        }
        else
        {
            write_eeprom(length);
            respondOK();
        }
        return;
    }
    comm.print((char)Resp_STK_UNKNOWN);
    return;
}

void AvrISP::eeprom_read_page(int length)
{
    // here again we have a word address
    // int start = here * 2;
    int x = 0;
    while (x < length)
    {
        // uint8_t ee = spi_transaction(0xA0, (addr >> 8) & 0xFF, addr & 0xFF, 0xFF);
        uint8_t ee = 0;
        buff2[x++] = ee;
    }
    return;
}

uint8_t AvrISP::flash_read(uint8_t hilo, int addr)
{
    return spi_transaction(0x20 + hilo * 8, (addr >> 8) & 0xFF, addr & 0xFF, 0);
}

void AvrISP::flash_read_page(int length)
{
    int x = 0;
    while (x < length)
    {
        buff2[x++] = flash_read(LOW, here);
        buff2[x++] = flash_read(HIGH, here);
        here++;
    }
}

void AvrISP::read_page()
{
    if (data_index < 3)
    {
        respondERR();
        return;
    }
    int length = getLength();
    char memtype = buff[2];
    if (memtype == 'F')
        flash_read_page(length);
    else if (memtype == 'E')
        eeprom_read_page(length);
    else
    {
        comm.print((char)Resp_STK_UNKNOWN);
        return;
    }
    respondBuffer(buff2, length);
}

void AvrISP::universal()
{
    if (data_index == 4)
    {
        uint8_t ch;
        ch = spi_transaction(buff[0], buff[1], buff[2], buff[3]);
        respondByte(ch);
    }
    else
        respondERR();
}

void AvrISP::read_signature()
{
    buff2[0] = spi_transaction(0x30, 0x00, 0x00, 0x00);
    buff2[1] = spi_transaction(0x30, 0x00, 0x01, 0x00);
    buff2[2] = spi_transaction(0x30, 0x00, 0x02, 0x00);
    respondBuffer(buff2, 3);
}

public:
// It seems ArduinoISP is based on the original STK500 (not v2)
// but implements only a subset of the commands.
void AvrISP::avrisp()
{
    if (!comm.available())
        return;
    uint8_t ch = getch();
    digitalWrite(2, !digitalRead(2));
    switch (avr_state)
    {
    case AVR_ERROR:
    case AVR_IDLE:
        if (ch == Sync_CRC_EOP)
        {
            avr_state = AVR_WAIT_CMD;
            data_index = 0;
            debug.println("got 'Sync_CRC_EOP' -> AVR_WAIT_CMD");
        }
        return;

    case AVR_WAIT_CMD:
        avr_cmd = ch;
        avr_state = AVR_GET_DATA;
        data_index = 0;
        debug.printf("got cmd: '%s' -> AVR_GET_DATA\r\n", GetCommandName(avr_cmd));
        return;

    case AVR_GET_DATA:
        if (ch == Sync_CRC_EOP)
        {
            avr_state = AVR_WAIT_CMD;
            debug.print("got 'Sync_CRC_EOP'");
            if (data_index > 0)
            {
                debug.print("\r\nData: ");
                // debug.logOutput(buff, data_index);

                comm.logDirection(CommDevice::PORT_OUTPUT);
                for (size_t i = 0; i < data_index; ++i)
                    debug.printf("%02x", (unsigned int)buff[i]);
                debug.print("\r\n");
            }
            data_index = 0;
        }
        else
        {
            if (data_index < sizeof(buff))
                buff[data_index++] = ch;
            else
            {
                respondERR();
                avr_state = AVR_ERROR;
                avr_cmd = 0;
                data_index = 0;
                debug.println("Got buffer overrun: -> AVR_ERROR");
            }
            return;
        }
    }

    // AVRISP_DEBUG("CMD 0x%02x", avr_cmd);
    switch (avr_cmd)
    {
        // Regain synchronization when sync is lost.
        // Host sends this command until Resp_STK_INSYNC is received.
    case Cmnd_STK_GET_SYNC: // 0x30
        error = 0;
        respondOK();
        break;

    case Cmnd_STK_GET_SIGN_ON: // 0x31
        respondBuffer((uint8_t *)F("AVR ISP"), 7);
        break;
    //Get the value of a valid parameter from the STK500 starterkit. If the parameter is not
    // used, the same parameter will be returned together with a Resp_STK_FAILED
    // response to indicate the error.
    case Cmnd_STK_GET_PARAMETER: // 0x41
        get_parameter(buff[0]);
        break;

    case Cmnd_STK_SET_DEVICE: // 0x42
        if (data_index == 20)
        {
            set_parameters();
            respondOK();
        }
        else
        {
            respondERR();
            avr_state = AVR_ERROR;
        }

        break;

    case Cmnd_STK_SET_DEVICE_EXT: // 0x45 - ignored
        if (data_index == 5)
            respondOK();
        else
        {
            respondERR();
            avr_state = AVR_ERROR;
        }
        break;

    case Cmnd_STK_ENTER_PROGMODE: // 0x50
        start_pmode();
        respondOK();
        break;

    case Cmnd_STK_LEAVE_PROGMODE: // 0x51
        error = 0;
        end_pmode();
        respondOK();
        delay(5);
        // comm.stop();
        // AVRISP_DEBUG("left progmode");

        break;

    case Cmnd_STK_LOAD_ADDRESS: // 0x55
        if (data_index == 2)
        {
            here = ((uint16_t)buff[0]) | ((uint16_t)buff[1] << 8);
            // AVRISP_DEBUG("here=0x%04x", here);
            respondOK();
        }
        else
            respondERR();
        break;

    case Cmnd_STK_UNIVERSAL: // 0x56
        universal();
        break;

    // Program one word in FLASH memory - not implemented
    case Cmnd_STK_PROG_FLASH: // 0x60
        if (data_index == 2)
            respondOK();
        else
            respondERR();
        break;

    // Program one byte in EEPROM memory - not implemented!
    case Cmnd_STK_PROG_DATA: // 0x61
        if (data_index == 1)
            respondOK();
        else
            respondERR();
        break;

    case Cmnd_STK_PROG_PAGE: // 0x64
        program_page();
        break;

    case Cmnd_STK_READ_PAGE: // 0x74
        read_page();
        break;

    case Cmnd_STK_READ_SIGN: // 0x75
        read_signature();
        break;

        // anything else we will return STK_UNKNOWN
    default:
        // AVRISP_DEBUG("?!?");
        error++;
        comm.print((char)Resp_STK_UNKNOWN);
        break;
    }
}

};

AvrISP avr_ISP;
