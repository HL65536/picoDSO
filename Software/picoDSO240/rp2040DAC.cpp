#include "rp2040DAC.h"

// MultipinDAC

void MultipinDAC::setRaw(u32 rawValue) // actually sets all pins at the same time (if it seems that it does not, it's the constructor's fault, which sets everything to LOW but sequentially)
{
    // u32 wanted = rawValue << firstPin;
    // u32 curState = gpio_get_all();
    // u32 flipThese = wanted ^ curState;
    // sio_hw->gpio_out = wanted & mask;
    // hw_xor_alias(sio_hw)->gpio_out = flipThese & mask;
    // gpio_xor_mask(flipThese & mask);
    gpio_put_masked(mask, rawValue << firstPin);
}

void MultipinDAC::setFilled8bit(u8 val)
{
    setRaw((u64(val) << pinCount) >> 8);
}

void MultipinDAC::setFraction(float between0and1, bool true0toVCCrange)
{
    if (between0and1 < 0)
    {
        between0and1 = 0;
    }
    if (true0toVCCrange)
    {
        between0and1 *= (u32(1) << pinCount) - 1;
        u32 raw = between0and1;
        setRaw(raw);
    }
    else
    {
        u8 reduce = 0;
        if (between0and1 >= 1)
        {
            between0and1 = 1;
            reduce = 1; // reduce by epsilon
        }
        between0and1 *= (u32(65536ull) * 256); // *= 2^24
        u32 raw24 = between0and1;
        raw24 -= reduce;
        setRaw((u64(raw24) << pinCount) >> 24);
    }
}

void MultipinDAC::setVoltage(float wantedOutput, float VCC)
{
    setFraction(wantedOutput / VCC);
}

MultipinDAC::MultipinDAC(u8 FirstPin, u8 PinCount) : firstPin(FirstPin), pinCount(PinCount)
{
    if (firstPin + pinCount > 30)
    {
        mask = 0; // invalid
    }
    else
    {
        u32 ones = 0;
        for (int i = 0; i < pinCount; i++)
        {
            ones <<= 1;
            ones++;
            pinMode(firstPin + i, OUTPUT);
        }
        mask = ones << firstPin;
    }
}

bool MultipinDAC::hasValidConfig()
{
    return mask != 0;
}
