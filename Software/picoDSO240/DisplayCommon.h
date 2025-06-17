#ifndef SRC_DISPLAYCOMMON_H_
#define SRC_DISPLAYCOMMON_H_

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include "shortNames.h"

class DisplayCommon
{
protected:
    virtual void deinit() = 0; // init can then be called again without causing problems
    virtual void reinit() = 0; // init but shorter and with reused parameters

    void switchPowerOn();  // only switches power pins, not comm pins
    void switchPowerOff(); // only switches power pins, not comm pins

public:
    void commonLoopStart(u8 textSize);

    bool isInitialized = false; // stays false if errors occur during init
    bool pwrPin2isGND;
    i16 powerPin1;
    i16 powerPin2;
    u16 displayWidth;
    u16 displayHeight;


    virtual void startDrawing(u8 textSize = 2) = 0;//clears screen + other setup work
    //call these in the main loop before/after drawing
    virtual void endDrawing() = 0;//makes sure the drawed things are actually displayed

    virtual void shutdown() = 0;// cut power to the display and disconnect communication (if possible)
    virtual void resume() = 0;// restore power and communication
    virtual void Init() = 0;// call this at the beginning of the application before anything else display-related

    virtual Adafruit_GFX &getGFX() = 0;

    DisplayCommon(u16 Width,u16 Height,bool pwrPin2isGnd,i16 pwrPin1,i16 pwrPin2);
};
#endif /* SRC_DISPLAYCOMMON_H_ */