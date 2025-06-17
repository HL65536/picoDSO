#include "oscilloscopeUtils.h"

// GraphDisplaySpace

GraphDisplaySpace::GraphDisplaySpace(u16 Height, u16 Width):
height(Height),width(Width)
{}

i32 GraphDisplaySpace::toY(float valFrom0to1, bool pixel0isOnTop)
{
    if (pixel0isOnTop)
    {
        valFrom0to1 = 1 - valFrom0to1;
    }
    float max = height - 1;
    float pixelVal=max*valFrom0to1;
    if(pixelVal<=-1)
    {
        return -1;
    }
    if(pixelVal<0)
    {
        return 0;
    }
    if(pixelVal>height)
    {
        return -1;
    }
    if(pixelVal>max)
    {
        return max;
    }
    return pixelVal;
}

// BandwidthExtenderRC

void BandwidthExtenderRC::reset()
{
    lastVal = ~(i32(0));
}

/*float BandwidthExtenderRC::calcStep(float filteredVoltage)
{
    if (lastVal == ~(i32(0)))
    {
        lastVal=filteredVoltage;
        return filteredVoltage;
    }
    else
    {
        float difDir=filteredVoltage-lastVal;
        float ret = lastVal + difDir * correctionMultiplier;
        lastVal=filteredVoltage;
        return ret;
    }
}*/

i32 BandwidthExtenderRC::calcStep(u16 voltage)
{
    if (lastVal == ~(i32(0)))
    {
        lastVal = voltage * i32(65536);
        return lastVal;
    }
    else
    {
        i32 difDir = voltage - lastVal;
        i32 ret = lastVal * 65536 + difDir * correctionMultiplier;
        lastVal = voltage;
        return ret;
    }
}
BandwidthExtenderRC::BandwidthExtenderRC(float sampleTimeNanoseconds, float picofarads, float kOhms, float tuningFactor)
{
    float timingConstant = sampleTimeNanoseconds / (tuningFactor * kOhms * picofarads);
    float chargeChangeFraction = 1 - pow(EULER, -timingConstant);
    correctionMultiplier = 65536.0f / chargeChangeFraction;
}

u32 Recording::getNumSamples()
{
    return rawLength / channels;
}
void Recording::allocateSpace(u32 maxBytesUsed, u8 numChannels, bool use16bit)
{
    deallocate();
    maxBytesUsed /= numChannels;
    maxBytesUsed *= numChannels; // make sure it's integer dividable by Channels
    if (use16bit)
    {
        dataPtr16 = new u16[maxBytesUsed / sizeof(u16)];
        dataPtr8 = nullptr;
    }
    else
    {
        dataPtr8 = new u8[maxBytesUsed];
        dataPtr16 = nullptr;
    }
    rawLength = maxBytesUsed / (use16bit ? sizeof(u16) : sizeof(u8));
    channels = numChannels;
}
void Recording::deallocate()
{
    if (dataPtr16 != NULL)
        delete[] dataPtr16;
    else if (dataPtr8 != NULL)
        delete[] dataPtr8;
    dataPtr16 = 0;
    dataPtr8 = 0;
}

bool Recording::is16bit()
{
    return dataPtr16 != 0;
}

u16 Recording::readIndex(u32 index,u8 channel)
{
    if (dataPtr8 != 0)
    {
        if (channel == 0)
        {
            return dataPtr8[index];
        }
        channel %= channels;
        u32 rawIndex = index * channels + channel;
        return dataPtr8[rawIndex];
    }
    else if (dataPtr16 != 0)
    {
        if (channel == 0)
        {
            return dataPtr16[index];
        }
        channel %= channels;
        u32 rawIndex = index * channels + channel;
        return dataPtr16[rawIndex];
    }
    else return 42;//in case nothing is allocated, return a number that arouses suspicion
}

Recording::Recording() : dataPtr8(nullptr), dataPtr16(nullptr), rawLength(0), channels(0) {}

Recording::~Recording()
{
    deallocate();
}