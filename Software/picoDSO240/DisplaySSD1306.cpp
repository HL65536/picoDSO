#include "DisplaySSD1306.h"

#ifdef ARDUINO_ARCH_RP2040
#include "rp2040Tools.h"
#endif

void DisplaySSD1306::startDrawing(u8 textSize)
{
    u8 *framebuf = getBuffer();

#ifdef ARDUINO_ARCH_RP2040
    memset(framebuf, 0, displayHeight * displayWidth / 8); // faster on 32bit arm
#else
    u8 *end = framebuf + displayHeight * displayWidth / 8; // faster on 8bit processor
    while (framebuf < end)
    {
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
        *framebuf++ = 0;
    }
#endif
    setTextColor(SSD1306_WHITE);
    commonLoopStart(textSize);
}

DisplaySSD1306::DisplaySSD1306(TwoWire *twi, i16 sdaPin, i16 sclPin,
                               bool PreventCodeContinuationIfInitFails,
                               u8 I2CAddr, bool isSecondaryDisplayOnBus, i16 PowerPin,
                               i16 GNDpin, u16 Width, u16 Height)
: Adafruit_SSD1306(Width, Height, twi, -1),
DisplayCommon(Width,Height,true,PowerPin,GNDpin),
isSecondaryOnBus(isSecondaryDisplayOnBus),
preventCodeContinuationIfInitFails(PreventCodeContinuationIfInitFails),
TWI(twi), SDApin(sdaPin), SCLpin(sclPin), addrI2C(I2CAddr)
{}
//TODO hier weitermachen
DisplaySSD1306::DisplaySSD1306(u8 overclockLevel, TwoWire *twi, i16 sdaPin, i16 sclPin,
                               bool PreventCodeContinuationIfInitFails,
                               u8 I2CAddr, bool isSecondaryDisplayOnBus, i16 PowerPin,
                               i16 GNDpin, u16 Width, u16 Height)
: Adafruit_SSD1306(Width, Height, twi, -1, overclockLevel * 100000UL, overclockLevel * 100000UL),
DisplayCommon(Width, Height, true, PowerPin, GNDpin),
isSecondaryOnBus(isSecondaryDisplayOnBus),
preventCodeContinuationIfInitFails(PreventCodeContinuationIfInitFails),
TWI(twi), SDApin(sdaPin), SCLpin(sclPin), addrI2C(I2CAddr)
{}

Adafruit_GFX & DisplaySSD1306::getGFX()
{
    return *this;
}

void DisplaySSD1306::deinit()
{
    if (buffer)
    {
        free(buffer);
        buffer = NULL;
    }
    if (wire)
    {
        if (!isSecondaryOnBus)
        {
            wire->end();
        }
    }
    isInitialized = false;
}

void DisplaySSD1306::reinit()
{
    if (!begin(SSD1306_SWITCHCAPVCC, addrI2C, true, !isSecondaryOnBus))
    {
        if (preventCodeContinuationIfInitFails)
        {
            // Serial.println(F("SSD1306 allocation failed")); // uncomment if serial should print error
            for (;;)
                ; // Don't proceed, loop forever
        }
    }
    isInitialized = true;
}

void DisplaySSD1306::shutdown()
{
    deinit();
    switchPowerOff();
}

void DisplaySSD1306::resume()
{
    switchPowerOn();
    reinit();
}

void DisplaySSD1306::Init()
{
    if (!isSecondaryOnBus)
    {
#ifdef ARDUINO_ARCH_RP2040
        if (SDApin >= 0)
        {
            TWI->setSDA(SDApin);
        }
        if (SCLpin >= 0)
        {
            TWI->setSCL(SCLpin);
        }
#endif
    }
    resume();
}

void DisplaySSD1306::endDrawing()
{
    display(); // member of Adafruit_SSD1306, writes framebuffer to display
}