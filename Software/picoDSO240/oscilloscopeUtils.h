
#ifndef SRC_oscilloscopeUtils_H_
#define SRC_oscilloscopeUtils_H_

#include "shortNames.h"
#include "measurementTools.h"

class GraphDisplaySpace//represents pixels of the given size
{
    u16 height;
    u16 width;
public:
    GraphDisplaySpace(u16 Height,u16 Width);
    i32 toY(float valFrom0to1,bool pixel0isOnTop=true); // returns y pixel position of a y-normalized graph point or -1 if outside the screen
};


class BandwidthExtenderRC
{
    //float lastVal=-16777216;//code for uninitialized
    i32 lastVal = ~(i32(0)); //code for uninitialized
public:
    //float correctionMultiplier;//also multiplies noise, unfortunately
    i32 correctionMultiplier;
    void reset();
    //float calcStep(float filteredVoltage);
    i32 calcStep(u16 voltage);//returns value as fixed point representation
    BandwidthExtenderRC(float sampleTimeNanoseconds, float picofarads, float kOhms, float tuningFactor = 1);
};

class Recording
{
public:
    //only one pointer is active at a time:
    volatile u8 * dataPtr8;//is 0 if not allocated
    volatile u16 * dataPtr16;//is 0 not allocated

    volatile u32 rawLength;//length in number of samples of all channels combined
    volatile u8 channels;

    u32 getNumSamples();

    void deallocate();

    bool is16bit();

    u16 readIndex(u32 index,u8 channel=0);//channel 0 means a raw index instead of one resolved to a specific channel

    void allocateSpace(u32 maxBytesUsed, u8 numChannels = 1, bool use16bit = false);

    Recording();
    ~Recording();
};

#endif /* SRC_oscilloscopeUtils_H_ */