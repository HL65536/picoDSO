#include "joystickButtons.h"


void JoystickButtons::evaluate()
{
    pinMode(P1, INPUT_PULLUP);
    pinMode(P2, OUTPUT);
    digitalWrite(P2, LOW);
    delayMicroseconds(5); // time calculated for pin to assume correct voltage under worst-case circumstances
    bool bothConnectedA = !digitalRead(P1);
    pinMode(P1, INPUT_PULLDOWN);
    digitalWrite(P2, HIGH);
    delayMicroseconds(5);
    bool bothConnectedB = digitalRead(P1);

    bool middleButton = bothConnectedA && bothConnectedB;
    i8 upDir = 0;
    i8 rightDir = 0;

    if (!middleButton) // logically mutually exclusive (middle button produces wrong values)
    {
        pinMode(P2, INPUT);
        delayMicroseconds(5);
        if (digitalRead(P1)) // if "up" is pressed
        {
            upDir = 1;
        }
        else // physically mutually exclusive
        {
            pinMode(P1, INPUT_PULLUP);
            delayMicroseconds(5);
            if (!digitalRead(P1)) // if "down" is pressed
            {
                upDir = -1;
            }
        }
        pinMode(P1, INPUT);
        pinMode(P2, INPUT_PULLDOWN);
        delayMicroseconds(5);
        if (digitalRead(P2))// if "right" is pressed
        {
            rightDir = 1;
        }
        else // physically mutually exclusive
        {
            pinMode(P2, INPUT_PULLUP);
            delayMicroseconds(5);
            if (!digitalRead(P2)) // if "left" is pressed
            {
                rightDir = -1;
            }
        }
    }
    if ((lastButton == false) && middleButton)
    {
        buttonEvent=true;
    }
    lastButton = middleButton;
    if ((lastRightDir == 0) && (rightDir != 0))
    {
        rightEvents+=rightDir;
    }
    lastRightDir = rightDir;
    if ((lastUpDir == 0) && (upDir != 0))
    {
        upEvents += upDir;
    }
    lastUpDir=upDir;
}

bool JoystickButtons::getButtonEvent(bool reevaluate)
{
    if (reevaluate)
        evaluate();
    bool ret = buttonEvent;
    buttonEvent = false;
    return ret;
}
i8 JoystickButtons::getRightEvents(bool reevaluate)
{
    if (reevaluate)
        evaluate();
    i8 ret = rightEvents;
    rightEvents = 0;
    return ret;
}
i8 JoystickButtons::getUpEvents(bool reevaluate)
{
    if (reevaluate)
        evaluate();
    i8 ret = upEvents;
    upEvents = 0;
    return ret;
}

JoystickButtons::JoystickButtons(u8 p1, u8 p2, i16 extraVCCpin, i16 extraGNDpin) :
P2(p2), P1(p1)
{
    if (extraVCCpin >= 0)
    {
        pinMode(extraVCCpin, OUTPUT);
        digitalWrite(extraVCCpin, HIGH);
    }
    if (extraGNDpin >= 0)
    {
        pinMode(extraGNDpin, OUTPUT);
        digitalWrite(extraGNDpin, LOW);
    }
}