#ifndef SRC_JOYSTICKBUTTONS_H_
#define SRC_JOYSTICKBUTTONS_H_

#include "shortNames.h"

class JoystickButtons//used for having directional buttons + one center/confirm button, all on just 2 pins + VCC + GND
{// requires ability to select all of these pin states on the same pin:
//  internal pullup, internal pulldown, drive high, drive low, floating

    u8 P2 = 18;//pin 2, connected to left/right
    u8 P1 = 19; // pin 1, connected to up/down

    i8 lastUpDir = 0;
    i8 lastRightDir = 0;
    bool lastButton:1 = false;
    bool buttonEvent:1 = false;
    i8 upEvents=0;
    i8 rightEvents=0;

public:
    void evaluate();
    bool getButtonEvent(bool reevaluate = false); // true if button was pressed
    i8 getUpEvents(bool reevaluate = false); // down events are negative
    i8 getRightEvents(bool reevaluate = false); // left events are negative

    JoystickButtons(u8 p1, u8 p2, i16 extraVCCpin=-1, i16 extraGNDpin=-1); // a dummy is never pressed and does not attach to the given pin
};

#endif // SRC_JOYSTICKBUTTONS_H_