#include <Arduino.h>
#include <command.h>

typedef struct
{
    const uint8_t cmd;
    const __FlashStringHelper * const name;
} command;

const char *GetCommandName(uint8_t c)
{
    const command commands[] = {
        {Cmnd_STK_GET_SYNC, F("Cmnd_STK_GET_SYNC")},
        {Cmnd_STK_GET_SIGN_ON, F("Cmnd_STK_GET_SIGN_ON")},
        {Cmnd_STK_RESET, F("Cmnd_STK_RESET")},
        {Cmnd_STK_SINGLE_CLOCK, F("Cmnd_STK_SINGLE_CLOCK")},
        {Cmnd_STK_STORE_PARAMETERS, F("Cmnd_STK_STORE_PARAMETER")},
        {Cmnd_STK_SET_PARAMETER, F("Cmnd_STK_SET_PARAMETER")},
        {Cmnd_STK_GET_PARAMETER, F("Cmnd_STK_GET_PARAMETER")},
        {Cmnd_STK_SET_DEVICE, F("Cmnd_STK_SET_DEVICE")},
        {Cmnd_STK_GET_DEVICE, F("Cmnd_STK_GET_DEVICE")},
        {Cmnd_STK_GET_STATUS, F("Cmnd_STK_GET_STATUS")},
        {Cmnd_STK_SET_DEVICE_EXT, F("Cmnd_STK_SET_DEVICE_EXT")},
        {Cmnd_STK_ENTER_PROGMODE, F("Cmnd_STK_ENTER_PROGMODE")},
        {Cmnd_STK_LEAVE_PROGMODE, F("Cmnd_STK_LEAVE_PROGMODE")},
        {Cmnd_STK_CHIP_ERASE, F("Cmnd_STK_CHIP_ERASE")},
        {Cmnd_STK_CHECK_AUTOINC, F("Cmnd_STK_CHECK_AUTOINC")},
        {Cmnd_STK_CHECK_DEVICE, F("Cmnd_STK_CHECK_DEVICE")},
        {Cmnd_STK_LOAD_ADDRESS, F("Cmnd_STK_LOAD_ADDRESS")},
        {Cmnd_STK_UNIVERSAL, F("Cmnd_STK_UNIVERSAL")},
        {Cmnd_STK_PROG_FLASH, F("Cmnd_STK_PROG_FLASH")},
        {Cmnd_STK_PROG_DATA, F("Cmnd_STK_PROG_DATA")},
        {Cmnd_STK_PROG_FUSE, F("Cmnd_STK_PROG_FUSE")},
        {Cmnd_STK_PROG_LOCK, F("Cmnd_STK_PROG_LOCK")},
        {Cmnd_STK_PROG_PAGE, F("Cmnd_STK_PROG_PAGE")},
        {Cmnd_STK_PROG_FUSE_EXT, F("Cmnd_STK_PROG_FUSE_EXT")},
        {Cmnd_STK_READ_FLASH, F("Cmnd_STK_READ_FLASH")},
        {Cmnd_STK_READ_DATA, F("Cmnd_STK_READ_DATA")},
        {Cmnd_STK_READ_FUSE, F("Cmnd_STK_READ_FUSE")},
        {Cmnd_STK_READ_LOCK, F("Cmnd_STK_READ_LOCK")},
        {Cmnd_STK_READ_PAGE, F("Cmnd_STK_READ_PAGE")},
        {Cmnd_STK_READ_SIGN, F("Cmnd_STK_READ_SIGN")},
        {Cmnd_STK_READ_OSCCAL, F("Cmnd_STK_READ_OSCCAL")},
        {Cmnd_STK_READ_FUSE_EXT, F("Cmnd_STK_READ_FUSE_EXT")},
        {Cmnd_STK_READ_OSCCAL_EXT, F("Cmnd_STK_READ_OSCCAL_EXT")},
        {0, F("<Unknown command>")}};

    int i = -1;
    while (commands[++i].cmd)
        if (commands[i].cmd == c)
            break;
    return (const char *)commands[i].name;
}