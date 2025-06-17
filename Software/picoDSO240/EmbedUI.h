#ifndef SRC_EMBEDUI_H_
#define SRC_EMBEDUI_H_

#include "shortNames.h"

#include "metric.h"

const TimeLight debounceTime = 0.02_s;


inline u16 FloatSettingToInt(float f)
{
    if(f>1970.0f)
    {
        return 2047;
    }
    if (f < 1.0f)
    {
        return 0;
    }
    u8 exp10 = 0;
    if (f >= 1000.0f)
    {
        f /= 1000.0f;
        exp10 = 3;
    }
    else if (f >= 100.0f)
    {
        f /= 100.0f;
        exp10 = 2;
    }
    else if (f >= 10.0f)
    {
        f /= 10.0f;
        exp10 = 1;
    }
    u16 ret;
    if(f>=4.995f)
    {
        f += 0.01f;//round correctly
        ret = f / 0.02f;
        ret += 150;//5->400; 10->650
    }
    else
    {
        f += 0.005f; //round correctly
        ret=f/0.01f;
        ret-=100;//1->0; 5->400
    }
    ret+=650*exp10;
    return ret;
}

inline float SettingIntToFloat(u16 ui)
{
    u8 exp10=ui/650;
    u16 num=ui%650;
    float ret;
    if(num>=400)
    {
        ret = num * 0.02f - 3.0f;//400->5; 650->10
    }
    else
    {
        ret = 1.0f + num * 0.01f;//0->1; 400->5
    }
    if(exp10==0)
    {
        return ret;
    }
    if(exp10==1)
    {
        return ret*10.0f;
    }
    if(exp10==2)
    {
        return ret*100.0f;
    }
    return ret*1000.0f;
}

class Button
{
    TimeLight lastChangeTimer;
    u8 pin;
    bool lastState:1; // true=button pressed
    bool event : 1;
    bool isDummy : 1;

public:
    void tick(TimeLight timeElapsed);
    bool state(); //true = currently pressed
    bool wasPressed(); //consumes event

    // does not consume the event:
    TimeLight pressedSince();  //returns time since press event in s or -1 if not currently pressed
    TimeLight releasedSince(); //returns time since press event in s or -1 if not currently released

    inline bool quickEval(TimeLight timeElapsed=0.04_s);//returns true if press event occured on standalone button
    void reset();
    Button(u8 pinID,bool IsDummy=false);//a dummy is never pressed and does not attach to the given pin
};

inline bool Button::quickEval(TimeLight timeElapsed)
{
    tick(timeElapsed);
    return wasPressed();
}

class Selection
{
public:
    u16 max:11; //number of choices to select, max: 2047
    u16 state:11; //what is currently selected
    u16 changeEvent : 1; //bool
    u16 allowOF : 1;     //bool
    void increment(u8 times,bool overrideOverflowProtection=false);
    void decrement(u8 times);

    void changeMax(u16 newMax); //max 2047!
    u16 getState();
    u16 getMax();
    bool hasChanged(); //consumes change event
    Selection(u16 Max, bool allowOverflow = true); //max 2047!
};

class SelectButtons
{
    Button &up;
    Button &down;
    u8 confirmed;//hold to speedup state
public:
    void tick(TimeLight timeElapsed,Selection& toSet, bool wasReset = 0);
    SelectButtons(Button &Up, Button &Down);
};

//purpose: counting newlines to print only a certain line
class LinePrinter : public Print
{
public:
    i16 newlines : 15;
    i16 hasPrinted : 1; //bool
    Print *forward = 0;
    size_t write(uint8_t) override;
    void setLineOutput(Print &output, i16 lineNum);
    void startLineCounting(); //active at construction

    LinePrinter();
};


#ifdef IS_VON_NEUMANN_ARCH
    #include "EmbedUI_vonNeumannArch.h"
#else // IS_HARVARD_ARCH
    #include "EmbedUI_harvardArch.h"
#endif


#endif /* SRC_EMBEDUI_H_ */