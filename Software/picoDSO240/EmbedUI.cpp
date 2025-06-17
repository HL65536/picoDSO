#include "EmbedUI.h"


Button::Button(u8 pinID,bool IsDummy):
pin(pinID),lastState(false),event(false),isDummy(IsDummy)
{
    if(!isDummy)
    {
        pinMode(pin, INPUT_PULLUP);
    }
}
void Button::tick(TimeLight timeElapsed)
{
    lastChangeTimer += timeElapsed;
    lastChangeTimer.overflowProtect();
    bool curState;
    if(isDummy)
    {
        curState=lastState;
    }
    else
    {
        curState = !digitalRead(pin); //low means pressed
    }
    if (curState == lastState)
    {
        return;//if steady-state, nothing else to do
    }
    if (lastChangeTimer < debounceTime) //if change came too quickly after last change,
    {
        return; // it is ignored because it is most likely button bounce
    }// if change persists, timer will go up sufficiently to continue:
    if (curState) //if button freshly pressed
    {
        event = true;
    }
    lastChangeTimer = 0;
    lastState = curState;
}
bool Button::wasPressed() //consumes event
{
    if (event)
    {
        event = false; //clear press event
        return true;
    }
    return false;
}

bool Button::state()
{
    return lastState;
}

TimeLight Button::pressedSince()
{
    if (lastState == true) //if currently pressed
    {
        return lastChangeTimer;
    }
    else
    {
        return TimeLight(-1);
    }
}
TimeLight Button::releasedSince()
{
    if (lastState == false) //if currently released
    {
        return lastChangeTimer;
    }
    else
    {
        return TimeLight(-1);
    }
}

void Button::reset()
{
    tick(1.0_s);
    lastChangeTimer=0;
}

//Selection
Selection::Selection(u16 Max, bool allowOverflow):
state(0),changeEvent(0),allowOF(allowOverflow?1:0)
{
    if(Max>=2048)
    {
        max=2047;
    }
    else
    {
        max=Max;
    }
}

u16 Selection::getState()
{
    return state;
}

u16 Selection::getMax()
{
    return max;
}

void Selection::increment(u8 times, bool overrideOverflowProtection)
{
    if (times != 0)
    {
        changeEvent = 1;
    }
    u16 newState = state; //expand from 11 bits
    newState += times;
    i16 items = max; //expand from 11 bits
    items++;         //items = max+1
    if (newState >= items)
    {
        if (allowOF || overrideOverflowProtection)
        {
            state = newState % items;
            return;
        }
        //else
        state = max; //cap instead of overflow
        return;
    }
    //if (newState < items)
    state = newState;
}
void Selection::decrement(u8 times)
{
    if (times != 0)
    {
        changeEvent = 1;
    }
    i16 newState = state; //expand from 11 bits
    newState -= times;
    if (newState < 0)
    {
        if (!allowOF)
        {
            state = 0; //cap instead of underflow
            return;
        }
        //if (allowOF && newState < 0) ...
        i16 items=max;//expand from 11 bits
        items++;//items = max+1
        newState %= items;
        if (newState < 0) //mod operation can have negative results
        {
            newState += items;
        }
    }
    // newState >= 0
    state = newState;
}

bool Selection::hasChanged()
{
    if (changeEvent)
    {
        changeEvent = 0; //clear event
        return true;
    }
    return false;
}

void Selection::changeMax(u16 newMax)
{
    if(newMax>2047)//max has only 11 bits
    {
        newMax=2047;
    }
    max = newMax;
    if (newMax >= state)
    {
        return;//everything still ok
    }
    increment(0); //correct overflow
}

//SelectButtons
SelectButtons::SelectButtons(Button &Up, Button &Down):
up(Up), down(Down),confirmed(0)
{}

void SelectButtons::tick(TimeLight timeElapsed, Selection &toSet, bool wasReset)
{
    u8 holdSpeedup=16;
    u8 max8 = toSet.getMax() / 8; //max 2047/8 = 255
    if(max8>holdSpeedup)
    {
        holdSpeedup=max8;
    }
    if (wasReset)
    {
        up.reset();
        down.reset();
        confirmed=0;
    }
    up.tick(timeElapsed);
    down.tick(timeElapsed);
    if (up.wasPressed())
    {
        toSet.increment(1);
        confirmed = 0;
    }
    if (down.wasPressed())
    {
        toSet.decrement(1);
        confirmed = 0;
    }
    if (up.state() && down.state())
    {
        return; //if both pressed, no hold speedup
    }
    if (holdSpeedup == 0)
    {
        return; //no hold speedup
    }
    TimeLight pressedSince = up.pressedSince() - 0.5_s;
    if(pressedSince>0.0_s)
    {
        u8 increments = u8(pressedSince.toFloat() * holdSpeedup);
        u8 dif=increments-confirmed;
        confirmed=increments;
        toSet.increment(dif);
        return;//both buttons pressed is already excluded
    }
    pressedSince = down.pressedSince() - 0.5_s;
    if (pressedSince > 0.0_s)
    {
        u8 increments = u8(pressedSince.toFloat() * holdSpeedup);
        u8 dif = increments - confirmed;
        confirmed = increments;
        toSet.decrement(dif);
    }
}



//LinePrinter
LinePrinter::LinePrinter():
newlines(0),hasPrinted(0)
{}
void LinePrinter::startLineCounting()
{
    newlines = 0;
    hasPrinted = 0;
    forward=0;
}
void LinePrinter::setLineOutput(Print& output,i16 lineNum)
{
    forward = &output;
    hasPrinted = 0;
    newlines = -lineNum;
}
size_t LinePrinter::write(uint8_t c)
{
    if (c == (uint8_t)'\n')
    {
        newlines++;
        return 1;
    }
    if(newlines!=0)
    {
        return 1;
    }
    //else
    if(forward)
    {
        hasPrinted = -1;
        return forward->write(c);
    }
    //else
    return 1;
}
