#ifndef SRC_RP2040PWM_H_
#define SRC_RP2040PWM_H_

#include "shortNames.h"

extern u8 syncPWMs;//stores which slices are waiting to be activated simultaneously (bits 0-7 corresond to slices 0-7)

class PWMslice
{
    u8 sliceNum;

public:
    void setClockDivider(float div);//fractional 8bits:4bits
    void setCounterMax(u16 periodMinus1);//double-buffered: takes effect next counter cycle
    void setPhaseCorrect(bool phaseCorrect);//"true" halves frequency
    void setOutputPolarities(bool invertEvenPin, bool invertOddPin);
    void setEvenPinInverted(bool inverted);
    void setOddPinInverted(bool inverted);
    void setPinInverted(u8 pin, bool inverted); // does not verify if "pin" is valid!
    void setCounterValue(u16 val);
    void start();//starts the counter and therefor the PWM output, also restarts after freeze() or stopAt0()
    void freeze();// stops without changing status of pins or counter
    void stopAt0();// stops while ensuring the pins are in duty 0% state
    void stopAt1();// stops while ensuring the pins are in duty 100% state
    void incrementCounter();//only works while a) running AND b) clkDiv>1
    void decrementCounter();//only works while running (just ignores next clock tick)
    void setEvenDuty(u16 val);
    void setOddDuty(u16 val);// duty val = number of clock cycles "on"
    void setPinDuty(u8 pin,u16 val);//does not verify if "pin" is valid!
    volatile u16 &refToEvenPinDuty();
    volatile u16 &refToOddPinDuty();
    volatile u32 &refToBothDuty();
    volatile u16 &refToPinDuty(u8 pin); // does not verify if "pin" is valid!
    volatile u32 &refToRawClkDivRegister();// 0x10 is a clock divider of 1; only 12 bits used
    volatile u32 &refToRawCounterRegister();// only 16 bits used

    //writes are double-buffered and change on counter value wrap
    volatile u32 &refToCounterMax();// counter runs to <= this value (not < !), only 16 bit are used, writing 65537 will result in a period of 2!!!!

    // if false, wait until the next true call, then all previously called are started
    void simultaneouslyEnable(bool startAllCalledSlicesNow);

    u32 getRawDreqID();//for use by DMA

    PWMslice(u8 pin, bool initPin = true);
    //even and odd pins have to match to same slice! (otherwise undefined behaviour!)
    PWMslice(u8 evenPin,u8 oddPin, bool initPins = true);
};

#endif /*SRC_RP2040PWM_H_*/