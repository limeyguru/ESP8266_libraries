#include <Arduino.h>
#include "led.h"

LED _leds[NUM_DIGITAL_PINS]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

// #define ALLOW_FLASH_INTERFACE_PINS

bool validPin(int pin)
{
#ifdef ALLOW_FLASH_INTERFACE_PINS
    return (pin >= 0 && pin < NUM_DIGITAL_PINS);
#else
    return (pin >= 0 && pin < NUM_DIGITAL_PINS) && !isFlashInterfacePin(pin);
#endif
}

void LED::select(bool select)
{
    if (!isValid())
    {
        selected = false;
        return;
    }
    if (select && !selected)
    {
        selected = true;
        state = true;
        last_blink = millis();
        pinMode(num, OUTPUT);
        digitalWrite(num, state ? LOW : HIGH);
        init = true;
    }
    else if (!select && selected)
    {
        selected = false;
        state = false;
        init = false;
        pinMode(num, INPUT);
    }
}
void LED::On()
{
    if (isValid())
    {
        state = true;
        digitalWrite(num, LOW);
    }
}
void LED::Off()
{
    if (isValid())
    {
        state = false;
        digitalWrite(num, HIGH);
    }
}
void LED::Flip()
{
    if (isValid() && selected)
    {
        state = !state;
        digitalWrite(num, !state);
    }
}
void LED::NextCyclePin()
{
    if (cycle_pin < MAX_DIGITAL_PIN && cycle_pin >= 0)
        ++cycle_pin;
    else
        cycle_pin = 0;
}
String LED::SelectedPins()
{
    String result("");
    for (int i = 0; i < NUM_DIGITAL_PINS; ++i)
    {
        if (_leds[i].selected)
            result = result + i + ", ";
    }
    if (result.length() > 2)
        result.remove(result.length() - 2, 2);
    else
        result = "(none)";
    return result;
}

bool LED::blink = false;
int LED::blink_time = 250;
int LED::cycle_pin = 0;

void leds()
{
    LED::NextCyclePin();
    LED &led = _leds[LED::cycle_pin];
    if (LED::blink)
    {
        int now = millis();
        if (led.isValid() && led.selected)
        {
            if (now >= led.last_blink + LED::blink_time)
            {
                led.last_blink = now;
                led.state = !led.state;
                digitalWrite(LED::cycle_pin, !led.state);
            }
        }
    }
    else if (led.isValid() && led.selected && digitalRead(led.num) == digitalRead(led.num))
        digitalWrite(LED::cycle_pin, !led.state);
}
