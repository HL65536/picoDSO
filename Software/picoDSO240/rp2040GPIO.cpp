#include "rp2040GPIO.h"

void GPIOpin::setLowestResistance()
{
    gpio_set_drive_strength(pinNumber, GPIO_DRIVE_STRENGTH_12MA);
}
bool GPIOpin::isLowestResistance()
{
    return gpio_get_drive_strength(pinNumber) == GPIO_DRIVE_STRENGTH_12MA;
}
void GPIOpin::setMaxDrive()
{
    setLowestResistance();
    setSlewRate(true);
}
bool GPIOpin::isMaxDrive()
{
    return isFastSlewRate() && isLowestResistance();
}
void GPIOpin::setPullResistors(bool pullUp, bool pullDown)
{
    gpio_set_pulls(pinNumber, pullUp, pullDown);
}

void GPIOpin::setSchmittTrigger(bool enableSchmittTrigger)
{
    gpio_set_input_hysteresis_enabled(pinNumber, enableSchmittTrigger);
}

void GPIOpin::setSlewRate(bool fast)
{
    gpio_set_slew_rate(pinNumber, fast ? GPIO_SLEW_RATE_FAST : GPIO_SLEW_RATE_SLOW);
}
bool GPIOpin::isFastSlewRate()
{
    return gpio_get_slew_rate(pinNumber) == GPIO_SLEW_RATE_FAST;
}

GPIOpin::GPIOpin(u8 pin) : pinNumber(pin)
{
}

GPIOpin pin0(0);
GPIOpin pin1(1);
GPIOpin pin2(2);
GPIOpin pin3(3);
GPIOpin pin4(4);
GPIOpin pin5(5);
GPIOpin pin6(6);
GPIOpin pin7(7);
GPIOpin pin8(8);
GPIOpin pin9(9);
GPIOpin pin10(10);
GPIOpin pin11(11);
GPIOpin pin12(12);
GPIOpin pin13(13);
GPIOpin pin14(14);
GPIOpin pin15(15);
GPIOpin pin16(16);
GPIOpin pin17(17);
GPIOpin pin18(18);
GPIOpin pin19(19);
GPIOpin pin20(20);
GPIOpin pin21(21);
GPIOpin pin22(22);
GPIOpin pin23(23);
GPIOpin pin24(24);
GPIOpin pin25(25);
GPIOpin pin26(26);
GPIOpin pin27(27);
GPIOpin pin28(28);
GPIOpin pin29(29);
GPIOpin pin30(30);
GPIOpin pin31(31);