#ifndef SRC_DisplayST7789_H_
#define SRC_DisplayST7789_H_

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Arduino.h>
#include "shortNames.h"
#include "DisplayCommon.h"

#define ST7789_WIDTH 240
#define ST7789_HEIGHT 240

class DisplayST7789 : public Adafruit_ST7789, public DisplayCommon
{
    // wrapper for Adafruit_ST7789 library for support for pinout change and VCC via I/O
protected:
    void deinit() override; // init can then be called again without causing problems
    void reinit() override; // init but shorter and with reused parameters

public:
    i16 DApin = -1;
    i16 CLpin = -1;

    i16 RSTpin = -1;
    i16 DCpin = -1;
    i16 BLpin = -1;

    i16 RXpinDummy = -1;

#ifdef ARDUINO_ARCH_RP2040
    SPIClassRP2040 *usedSPI = 0;
    DisplayST7789(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1 = -1, i16 PowerPin2 = -1, i16 BL_pin = -1, i16 RX_pin_dummy = -1, SPIClassRP2040 *spi = &SPI);
#else
    SPIClass *usedSPI = 0;
    // code for different boards here
    DisplayST7789(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1 = -1, i16 PowerPin2 = -1, i16 BL_pin = -1, i16 RX_pin_dummy = -1, SPIClass *spi = &SPI);
#endif

    void startDrawing(u8 textSize = 2) override; // clears screen + other setup work
    // call these in the main loop before/after drawing
    void endDrawing() override; // makes sure the drawed things are actually displayed

    void shutdown() override; // deinitializes display and cuts power if pin is available
    void resume() override;   // reinitializes display and returns power if pins are available, this needs some time
    void Init() override;

    Adafruit_GFX &getGFX() override;
    //void init240(u8 daPin, u8 clPin);
};
#endif /* SRC_DisplayST7789_H_ */