#include "rp2040clock.h"

#include "hardware/clocks.h"
#include "pico/stdlib.h"

ClockspeedConfig::ClockspeedConfig(u16 Mult, u8 PD1, u8 PD2) : mult(Mult), pd1(PD1), pd2(PD2)
{
}

float ClockspeedConfig::getAchievedMHz()
{
    u32 HzRef = getReferenceClockspeed();
    HzRef *= mult;
    float div = pd1 * pd2 * 1000000ul; // Hz->MHz
    return HzRef / div;
}

void ClockspeedConfig::apply()
{
    u32 vcs_freq_hz = mult * getReferenceClockspeed();
    set_sys_clock_pll(vcs_freq_hz, pd1, pd2);
}
ClockspeedConfig findClockspeedConfig(float tryMHz, bool preferLowJitterOverLowPowerDraw, bool allowBelowNewSpecs)
{                                     // TODO fix
    ClockspeedConfig best(125, 4, 3); // should be valid if no other config is found
    float bestDeviation = 10000.0f;
    u32 HzRef = getReferenceClockspeed();
    if (preferLowJitterOverLowPowerDraw)
    {
        for (u32 mult = 320; mult >= 16; mult--)
        {
            if ((HzRef * mult) > 1600000000ul) // vco_max
            {
                continue;
            }
            if ((HzRef * mult) < (allowBelowNewSpecs ? 400000000ul : 750000000ul)) // vco_min
            // VALUE HAS BEEN CHANGED: v1.8 of datasheet and v1.3.1 of sdk say that vco_min should be 750Mhz, change here?
            {
                break; // will only get lower, so search is finished
            }
            for (u32 pd2 = 1; pd2 <= 7; pd2++)
            {
                for (u32 pd1 = pd2; pd1 <= 7; pd1++) // always have pd1>=pd2, as the other way round leads to the same clockspeed and does not need to be tested
                {
                    ClockspeedConfig test(mult, pd1, pd2);
                    float result = test.getAchievedMHz();
                    float dif = result - tryMHz;
                    if (dif < 0)
                    {
                        dif = -dif;
                    }
                    if (dif < bestDeviation)
                    {
                        bestDeviation = dif;
                        best = test;
                    }
                }
            }
        }
    }
    else
    {
        for (u32 mult = 16; mult <= 320; mult++)
        {
            if (HzRef * mult < (allowBelowNewSpecs ? 400000000ul : 750000000ul)) // vco_min
            {
                continue;
            }
            if (HzRef * mult > 1600000000ul) // vco_max
            {
                break; // will only rise more, so search is finished
            }
            for (u32 pd2 = 1; pd2 <= 7; pd2++)
            {
                for (u32 pd1 = pd2; pd1 <= 7; pd1++) // always have pd1>=pd2, as the other way round leads to the same clockspeed and does not need to be tested
                {
                    ClockspeedConfig test(mult, pd1, pd2);
                    float result = test.getAchievedMHz();
                    float dif = result - tryMHz;
                    if (dif < 0)
                    {
                        dif = -dif;
                    }
                    if (dif < bestDeviation)
                    {
                        bestDeviation = dif;
                        best = test;
                    }
                }
            }
        }
    }
    return best;
}

u32 getCPUClockspeed()
{
    return clock_get_hz(clk_sys);
}

bool setCPUClockspeed(u32 kHz)
{
    return set_sys_clock_khz(kHz, false);
}

u32 getReferenceClockspeed()
{
    return clock_get_hz(clk_ref);
}

