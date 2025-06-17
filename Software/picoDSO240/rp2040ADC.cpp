#include "rp2040ADC.h"

float globalSolderedReferenceVoltage = 3.27f; // 3.3V 150µA through 200 Ohm

// ADC overclocking:
// no artifacts up to 1MSa / s
// light artifacts up to 2MSa / s
// strong artifacts at 3MSa / s, unusable in certain voltage range
void setADCclockBoundToCPUclock(bool enable)
{
    volatile u32 *vreg = (volatile u32 *)(CLOCKS_BASE + 0x60);
    if (enable)
    {
        *vreg = 0x820; // https://forums.raspberrypi.com/viewtopic.php?t=340691
    }
    else
    {
        *vreg = 0x800; // bit 7:5 AUXSRC Selects the auxiliary clock source, will glitch when switched
        // 0x0 → clksrc_pll_usb
        // 0x1 → clksrc_pll_sys
        // 0x2 ... 0x5 → [...]
    }
    delayMicroseconds(200); // I hope this makes whatever clock glitch resolve itself
}

void prepareADC(u8 measureTarget)
{
    if (measureTarget >= 26) // for when the user uses GPIO numbers instead of ADC numbers
    {
        measureTarget -= 26;
    }
    if (measureTarget <= 3)
    {
        adc_gpio_init(measureTarget + 26);
    }
    if (measureTarget == ADC_TEMP)
    {
        adc_set_temp_sensor_enabled(true);
    }
    adc_select_input(measureTarget);
}
u16 singleConversion()
{
    return adc_read(); // TODO correct for fab error (see Errata RP2040-E11)
}
u16 readADC(u8 measureTarget)
{
    prepareADC(measureTarget);
    return singleConversion();
}
u32 readADCmulti(u8 measureTarget, u16 sampleCount)
{
    prepareADC(measureTarget);
    u32 ret = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        ret += singleConversion();
    }
    return ret;
}
void prepareADCrapid(u8 measureTarget, bool shift8bit, bool includeErrorFlag)
{
    prepareADC(measureTarget);
    adc_fifo_setup(true, false, 4, includeErrorFlag, shift8bit);

    stopFreeRunningMode(); // just in case it is already running, to not corrupt data
    // it also cleans up residual data (if any), as to not confuse it for new data
}
void prepareADCforDMA(u8 measureTarget, bool shift8bit, bool includeErrorFlag)
{
    prepareADC(measureTarget);
    adc_fifo_setup(true, true, 1, includeErrorFlag, shift8bit);

    stopFreeRunningMode(); // just in case it is already running, to not corrupt data
    // it also cleans up residual data (if any), as to not confuse it for new data
}
u8 toMask(u8 adcNum)
{
    if (adcNum >= 26) // for when the user uses GPIO numbers instead of ADC numbers
    {
        adcNum -= 26;
    }
    if (adcNum > 4) // invalid input
    {
        return 0;
    }
    return 1 << adcNum;
}
void prepareADCroundRobinRapid(u8 mask, bool shift8bit, bool includeErrorFlag)
{
    if (mask == 0 || mask > 0x1F)
    {
        return; // invalid mask value
    }
    for (u8 i = 0; i <= 4; i++)
    {
        if (mask & toMask(i))
        {
            prepareADC(i);
        }
    }
    adc_set_round_robin(mask);
    singleConversion(); // drain manually set channel entry and select first channel entry from round robin
    adc_fifo_setup(true, true, 1, includeErrorFlag, shift8bit);
    stopFreeRunningMode(false); // just in case it is already running, to not corrupt data
    // it also cleans up residual data (if any), as to not confuse it for new data
}

void prepareADCroundRobinForDMA(u8 mask, bool shift8bit, bool includeErrorFlag)
{
    if (mask == 0 || mask > 0x1F)
    {
        return; // invalid mask value
    }
    for (u8 i = 0; i <= 4; i++)
    {
        if (mask & toMask(i))
        {
            prepareADC(i);
        }
    }
    adc_set_round_robin(mask);
    singleConversion(); // drain manually set channel entry and select first channel entry from round robin
    adc_fifo_setup(true, false, 4, includeErrorFlag, shift8bit);
    stopFreeRunningMode(false); // just in case it is already running, to not corrupt data
    // it also cleans up residual data (if any), as to not confuse it for new data
}

void startFreeRunningMode()
{
    // clearMissedSampleFlag(); //should only apply to this run
    adc_run(true);
}

bool stopFreeRunningMode(bool disableRoundRobin)
{
    adc_run(false);
    bool missed = missedOneSample();
    adc_fifo_drain();
    if (disableRoundRobin)
    {
        adc_set_round_robin(0); // disable round robin if enabled, otherwise does nothing
    }
    clearMissedSampleFlag();
    return missed;
}

bool missedOneSample()
{
    return adc_hw->fcs & ADC_FCS_OVER_BITS;
}

void clearMissedSampleFlag()
{
    hw_clear_alias(adc_hw)->fcs = ADC_FCS_OVER_BITS;
}

u32 readADCmultiRapid(u8 measureTarget, u16 sampleCount)
{
    prepareADCrapid(measureTarget); // TODO discard errors:  , false, true);
    startFreeRunningMode();
    u32 ret = 0;
    for (u16 i = 0; i != sampleCount; i++)
    {
        ret += adc_fifo_get_blocking();
    }
    stopFreeRunningMode();
    return ret;
}

float readVoltage(u8 measureTarget, u16 samples)
{
    float ret = readADCmulti(measureTarget, samples);

    if (samples == 0)
    {
        samples = 1; // div by zero protection
    }
    ret /= u32(samples) * u32(1024 * 4);
    ret *= globalSolderedReferenceVoltage;
    return ret;
}

u16 readVsysRaw16()
{
    return readADCmulti(ADC_VSYS, 16);
}

float readVsys(u16 samples)
{
    return readVoltage(ADC_VSYS, samples) * 3; // has 200K - 100K voltage divider
}

ADC_INIT_SINGLETON::ADC_INIT_SINGLETON()
{
    adc_init();
}
volatile ADC_INIT_SINGLETON THE_ADC_INIT_SINGLETON;
