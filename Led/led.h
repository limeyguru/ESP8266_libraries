#pragma once

bool validPin(int pin);

class LED
{
public:
    LED(int n) : num(n) {}
    const int num;
    bool selected = false;
    bool state = false;
    bool init = false;
    int last_blink = 0;

    bool isValid()  { return validPin(num); }
    void select(bool = true);
    void On();
    void Off();
    void Flip();

    static int blink_time;
    static int cycle_pin;
    static bool blink;
    static void NextCyclePin();
    static String SelectedPins();
};

extern LED _leds[NUM_DIGITAL_PINS];
#define MAX_DIGITAL_PIN (NUM_DIGITAL_PINS - 1)
#define MIN_BLINK 100
#define MAX_BLINK ULONG_MAX

void leds();
