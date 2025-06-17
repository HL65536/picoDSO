#include "DisplayCommon.h"

#ifdef ARDUINO_ARCH_RP2040
#include "rp2040Tools.h"
#endif
void DisplayCommon::switchPowerOn()
{
    bool needTime = false;
    if (powerPin2 > -1)//16
    {
        needTime = true;
        noInterrupts();                                     // prevent potentially wrong polarity state lasting any significant time
        digitalWrite(powerPin2, pwrPin2isGND ? LOW : HIGH); // on some chips, this prevents wrong polarity state from ever occurring
        pinMode(powerPin2, OUTPUT);
        digitalWrite(powerPin2, pwrPin2isGND ? LOW : HIGH);
        interrupts();
#ifdef ARDUINO_ARCH_RP2040
        GPIOpin(powerPin2).setLowestResistance();
#endif
    }
    if (powerPin1 > -1)
    {
        needTime=true;
        noInterrupts(); // prevent potentially wrong polarity state lasting any significant time
        digitalWrite(powerPin1, HIGH);//on some chips, this prevents wrong polarity state from ever occurring
        pinMode(powerPin1, OUTPUT);
        digitalWrite(powerPin1, HIGH);
        interrupts();
#ifdef ARDUINO_ARCH_RP2040
        GPIOpin(powerPin1).setLowestResistance();
#endif
    }
    if(needTime)
    {
        delay(100);
    }
}
void DisplayCommon::switchPowerOff()
{
    if(powerPin1 != -1)
    {
        pinMode(powerPin1, INPUT);
    }
    if(powerPin2 != -1)
    {
        pinMode(powerPin2, INPUT);
    }
}

void DisplayCommon::commonLoopStart(u8 textSize)
{
    Adafruit_GFX& gfx=getGFX();
    gfx.setTextSize(textSize);
    gfx.setCursor(0, 0);
}

DisplayCommon::DisplayCommon(u16 Width, u16 Height, bool pwrPin2isGnd, i16 pwrPin1, i16 pwrPin2):
pwrPin2isGND(pwrPin2isGnd),powerPin1(pwrPin1),powerPin2(pwrPin2),displayWidth(Width),displayHeight(Height)
{}