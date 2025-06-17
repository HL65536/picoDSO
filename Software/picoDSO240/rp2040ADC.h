#ifndef SRC_RP2040ADC_H_
#define SRC_RP2040ADC_H_

#include "shortNames.h"
#include "hardware/adc.h"

#define ADC0 0
#define ADC1 1
#define ADC2 2
#define ADC3 3


#define ADC_VSYS 3
// vsys/3 will be measured on raspberry pi pico on pin ADC3

#define ADC_TEMP 4

extern float globalSolderedReferenceVoltage;
// can be changed by user to fit the adc reference voltage; defaults to raspberry pi pico fixed reference

void prepareADC(u8 measureTarget);
u16 singleConversion(); // reads a single raw value from previously prepared ADC channel
u16 readADC(u8 measureTarget);
u32 readADCmulti(u8 measureTarget, u16 sampleCount = 32); // smart, blocking, but does not convert to voltage
void prepareADCrapid(u8 measureTarget, bool shift8bit = false, bool includeErrorFlag = false);
void prepareADCforDMA(u8 measureTarget, bool shift8bit = false, bool includeErrorFlag = false);
u8 toMask(u8 adcNum);
void prepareADCroundRobinRapid(u8 mask, bool shift8bit = false, bool includeErrorFlag = false);
void prepareADCroundRobinForDMA(u8 mask, bool shift8bit = false, bool includeErrorFlag = false);

void startFreeRunningMode();
bool stopFreeRunningMode(bool disableRoundRobin = true); // returns true if fifo has overflowed at any point of the capture run

// 2 helper functions, no need to call these on your own:
bool missedOneSample();       // if fifo has overflowed
void clearMissedSampleFlag(); // clears the flag

u32 readADCmultiRapid(u8 measureTarget, u16 sampleCount = 32); // smart, blocking, but does not convert to voltage; uses free running mode and FIFO
float readVoltage(u8 measureTarget, u16 samples = 32);
u16 readVsysRaw16();              // raw ADC*16 output (fills full 16bits)
float readVsys(u16 samples = 32); // V

// ADC overclocking:
// no artifacts up to 1MSa / s
// light artifacts up to 2MSa / s
// strong artifacts at 3MSa / s, unusable in certain voltage range
void setADCclockBoundToCPUclock(bool enable); // false means bound to USB clock of 48MHz
// sample rate = clockspeed / 96

class ADC_INIT_SINGLETON
{
public:
    ADC_INIT_SINGLETON(); // makes sure adc gets automatically initialized without needing the arduino stuff
};
#endif /* SRC_RP2040ADC_H_*/