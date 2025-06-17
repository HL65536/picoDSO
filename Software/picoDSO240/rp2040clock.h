#ifndef SRC_RP2040CLOCK_H_
#define SRC_RP2040CLOCK_H_

#include "shortNames.h"

u32 getReferenceClockspeed(); // Hz
u32 getCPUClockspeed(); // Hz

bool setCPUClockspeed(u32 kHz); // invalid returns false
// some valid frequencies are not working (treated as invalid), use findClockspeedConfig() instead to use these
// TODO check if valid frequencies are still refused with new clock specs
// known to work (MHz): 24,48,60,96,100,120,133,192,240

class ClockspeedConfig
{
    u16 mult;
    u8 pd1;
    u8 pd2;

public:
    float getAchievedMHz();
    void apply();

    ClockspeedConfig(u16 Mult, u8 PD1, u8 PD2);
};


// TODO is it fixed now? needs testing
ClockspeedConfig findClockspeedConfig(float tryMHz, bool preferLowJitterOverLowPowerDraw = true, bool allowBelowNewSpecs = false);
// prefer low power makes up to 5% power consumption difference (measured at 120MHz)
// valid frequency examples: 10MHz,
// 12MHz, 13MHz, ..., 135MHz, 136MHz,
// 138MHz, 140MHz, ..., 270MHz, 272MHz,
// 276MHz, 280MHz, ..., >500MHz, but that is too fast anyway
// expected overclocking results:
// https://www.youtube.com/watch?v=G2BuoFNLoDM


#endif /*SRC_RP2040CLOCK_H_*/