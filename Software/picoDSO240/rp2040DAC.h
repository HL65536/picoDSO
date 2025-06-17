#ifndef SRC_RP2040DAC_H_
#define SRC_RP2040DAC_H_

#include "shortNames.h"

// code for a software-DAC using multiple pins in R2R or exponential resistor configuration

class MultipinDAC
{
    u8 firstPin;
    u8 pinCount;
    u32 mask;

public:
    MultipinDAC(u8 FistPin, u8 PinCount); // firstPin will be LSB, firstPin+pinCount-1 will be MSB; max supported: 24bit

    bool hasValidConfig(); // false if passed init params are invalid (e.g. start at pin 28 and allocate 8 pins);
    void setFraction(float between0and1, bool true0toVCCrange = false);
    void setVoltage(float wantedOutput, float VCC = 3.3f);
    void setRaw(u32 rawValue);  // LSB of val = LSB of DAC, MSB depends on pinCount
    void setFilled8bit(u8 val); // MSB of val = MSB of DAC, LSB depends on pinCount
};

#endif /*SRC_RP2040DAC_H_*/