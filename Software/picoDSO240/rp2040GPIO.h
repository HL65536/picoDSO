#ifndef SRC_RP2040GPIO_H_
#define SRC_RP2040GPIO_H_

#include "shortNames.h"

class GPIOpin
{
    u8 pinNumber;

public:
    void setLowestResistance();
    bool isLowestResistance();
    void setMaxDrive(); // both low resistance and high slew rate
    bool isMaxDrive();

    void setPullResistors(bool pullUp, bool pullDown);
    // setting both pulls enables a "bus keep" function, i.e. a weak pull to whatever is current high/low state of GPIO.

    void setSchmittTrigger(bool enableSchmittTrigger);

    void setSlewRate(bool fast);
    bool isFastSlewRate();

    // TODO add:
    // enum gpio_function gpio_get_function (uint gpio)
    // static bool gpio_is_pulled_up (uint gpio)
    // static bool gpio_is_pulled_down (uint gpio)
    // output overrides


    GPIOpin(u8 pin);
};

extern GPIOpin pin0;
extern GPIOpin pin1;
extern GPIOpin pin2;
extern GPIOpin pin3;
extern GPIOpin pin4;
extern GPIOpin pin5;
extern GPIOpin pin6;
extern GPIOpin pin7;
extern GPIOpin pin8;
extern GPIOpin pin9;
extern GPIOpin pin10;
extern GPIOpin pin11;
extern GPIOpin pin12;
extern GPIOpin pin13;
extern GPIOpin pin14;
extern GPIOpin pin15;
extern GPIOpin pin16;
extern GPIOpin pin17;
extern GPIOpin pin18;
extern GPIOpin pin19;
extern GPIOpin pin20;
extern GPIOpin pin21;
extern GPIOpin pin22;
extern GPIOpin pin23;
extern GPIOpin pin24;
extern GPIOpin pin25;
extern GPIOpin pin26;
extern GPIOpin pin27;
extern GPIOpin pin28;
extern GPIOpin pin29;
extern GPIOpin pin30;
extern GPIOpin pin31;

#endif /*SRC_RP2040GPIO_H_*/