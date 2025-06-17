
#include "measurementTools.h"

// VoltageMapping

float VoltageMapping::map12bit(u16 rawVal)
{
    rawVal &= 0x0FFF; //isolate 12 bit (discard error bit)
    return (rawVal * preMult) * postAdd;
}
float VoltageMapping::map8bit(u8 rawVal)
{
    u16 to12bit = rawVal;
    to12bit <<= 4;
    to12bit += 2; // necessary/correct?
    return map12bit(to12bit);
}
float VoltageMapping::map(u16 rawVal)
{
    return map12bit(rawVal);
}
float VoltageMapping::map(u8 rawVal)
{
    return map8bit(rawVal);
}

i32 VoltageMapping::map12bitFixed(u16 rawVal)
{
    i32 raw = rawVal & 0x0FFF; //isolate 12 bit (discard error bit)
    return (raw * preMultFixed) + postAddFixed;
}
i32 VoltageMapping::map8bitFixed(u8 rawVal)
{
    i16 to12bit = rawVal;
    to12bit <<= 4;
    to12bit += 2; // necessary/correct?
    return map12bitFixed(to12bit);
}
i32 VoltageMapping::mapFixed(u16 rawVal)
{
    return map12bitFixed(rawVal);
}
i32 VoltageMapping::mapFixed(u8 rawVal)
{
    return map8bitFixed(rawVal);
}

void VoltageMapping::convertToFixedPoint()
{
    preMultFixed = toFixedPtRepr(preMult);
    postAddFixed = toFixedPtRepr(postAdd);
}

VoltageMapping::VoltageMapping(float upperResistor, float lowerResistor, float offset, float vref)
{
    preMult = vref / 4096.0f;
    preMult *= (upperResistor + lowerResistor) / lowerResistor;
    postAdd = offset;
    convertToFixedPoint();
}

i32 toFixedPtRepr(float f)
{
    return f * 65536;
}
float fromFixedPtRepr(i32 i)
{
    return i / 65536.0f;
}

// ProgressCounter

ProgressCounter::ProgressCounter( float ProgressSize, bool AllowPositive, bool AllowNegative, float Hysteresis, i64 initialProgress, float initialFraction):
progressSize(ProgressSize),progressFraction(initialFraction),progress(initialProgress),allowPositive(AllowPositive),allowNegative(AllowNegative),hysteresis(Hysteresis/progressSize)
{}

inline ProgressCounter ProgressCounter::operator+(const ProgressCounter &other) const
{
    ProgressCounter ret(progressSize,true,true,0,progress+other.progress,progressFraction);
    ret.addProgress(other.progressFraction);
    return ret;
}
inline ProgressCounter ProgressCounter::operator-(const ProgressCounter &other) const
{
    ProgressCounter ret(progressSize, true, true, 0, progress - other.progress, progressFraction);
    ret.addProgress(-other.progressFraction);
    return ret;
}

void ProgressCounter::addProgress(float progressInHumanReadableUnit) volatile
{
    progressFraction += progressInHumanReadableUnit / progressSize;
    i64 newProgress = (i64)progressFraction;

    progressFraction -= newProgress;

    uncommittedProgress += newProgress;
    i64 hyst = hysteresis;
    if (uncommittedProgress > hyst)
    {
        i64 diff = uncommittedProgress - hyst;
        uncommittedProgress = hyst;
        if (allowPositive)
            progress += diff;
    }
    if (uncommittedProgress < -hyst)
    {
        i64 diff = uncommittedProgress + hyst; // - (-hyst)
        uncommittedProgress = -hyst;
        if (allowNegative)
            progress += diff;
    }
}

float ProgressCounter::getTotal()
{
    float floatProgress = progress;
    floatProgress += progressFraction;
    return progressSize * floatProgress;
}

// MultiplexerX8
MultiplexerX8::MultiplexerX8(u8 Pin1, u8 Pin2, u8 pin4, u16 waitMicroseconds) : pin1(Pin1), pin2(Pin2), pin4(pin4), waitTime(waitMicroseconds)
{
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin4, OUTPUT);
}

void MultiplexerX8::select(u8 channel)
{
    digitalWrite(pin1, channel & 1 ? HIGH : LOW);
    digitalWrite(pin2, channel & 2 ? HIGH : LOW);
    digitalWrite(pin4, channel & 4 ? HIGH : LOW);
    delayMicroseconds(waitTime);//wait for channels to switch
}