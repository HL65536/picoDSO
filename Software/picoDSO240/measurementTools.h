
#include "shortNames.h"

class VoltageMapping //represents mapping from raw adc values to real-life voltage/current/...
{
    float preMult;
    float postAdd;
    i32 preMultFixed;
    i32 postAddFixed;

    void convertToFixedPoint();//takes float values and calculates fixed point values from them

public:

    float map8bit(u8 rawVal);
    float map12bit(u16 rawVal);
    float map(u8 raw8bitVal);
    float map(u16 raw12bitVal);

    //16:16 fixed point representation, is significantly faster but has limited accuracy&range
    i32 map8bitFixed(u8 rawVal);
    i32 map12bitFixed(u16 rawVal);
    i32 mapFixed(u8 rawVal);
    i32 mapFixed(u16 rawVal);

    //TODO more constructors
    VoltageMapping(float upperResistor,float lowerResistor,float offset=0,float vref=3.27f);//for standard voltage divider with optional offset (e.g. by zener diode or VCC/2 reference)
};


//TODO make evaluatable at compile time
i32 toFixedPtRepr(float f);
float fromFixedPtRepr(i32 i);


class ProgressCounter//for mAh, kWh or similar things that accumulate
{
    float progressSize;//how much a progress of 1 is in the human-readable unit e.g. progress of 1 = 0.001mAh; should be small enough that the smallest expected progress to be added is at least half that size

    volatile i64 uncommittedProgress=0;

public:
    volatile float progressFraction; // additional fraction of a full progress point, therefore <1

    volatile i64 progress;

    //String unit;//base unit of the human-readable unit, e.g. "Ah" for mAh //TODO figure out how to autoprint, then re-enable
    bool allowPositive;
    bool allowNegative;//if only negative progress is allowed, 
    float hysteresis;//if only positive/negative progress allowed, enables getting rid of unwanted progress due to noise in the progress capturing system
    void addProgress(float progressInHumanReadableUnit) volatile;

    //these only work if they use the same unit and progressSize:
    ProgressCounter operator+(const ProgressCounter &other) const;
    ProgressCounter operator-(const ProgressCounter &other) const;

    float getTotal();//gets the total progress in human readable units, within the limited accuracy of floats

    ProgressCounter(float ProgressSize, bool AllowPositive = true, bool AllowNegative = true, float Hysteresis = 0, i64 initialProgress = 0, float initialFraction = 0);
};

//TODO separate volatile / non-volatile variant

class MultiplexerX8
{
    u8 pin1;
    u8 pin2;
    u8 pin4;
    u16 waitTime;
public:
    MultiplexerX8(u8 Pin1,u8 Pin2,u8 pin4,u16 waitMicroseconds=1);
    void select(u8 channel);
};