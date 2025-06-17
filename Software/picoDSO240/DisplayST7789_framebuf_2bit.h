#ifndef SRC_DisplayST7789_FRAMEBUF_2BIT_H_
#define SRC_DisplayST7789_FRAMEBUF_2BIT_H_

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "DisplayST7789.h"
#include <Arduino.h>
#include "shortNames.h"
#include "DisplayCommon.h"

// NEEDS 15KB RAM

#define COLOR2BIT_BLACK 0
#define COLOR2BIT_GREEN 1
#define COLOR2BIT_MAGENTA 2
#define COLOR2BIT_WHITE 3
extern u16 colorLookup[];

class VirtualCanvasST7789_2bit : public Adafruit_GFX
{
public:
    u32 framebuf[ST7789_HEIGHT * ST7789_WIDTH / 16]; // 16 pixels in u32

    //for performance reasons, the color argument is the index of the color (as defined above), not the color itself
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    VirtualCanvasST7789_2bit();
};

class DisplayST7789_2bit : public DisplayST7789
{
public:
    VirtualCanvasST7789_2bit canvas;


#ifdef ARDUINO_ARCH_RP2040
    DisplayST7789_2bit(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1 = -1, i16 PowerPin2 = -1, i16 BL_pin = -1, i16 RX_pin_dummy = -1, SPIClassRP2040 *spi = &SPI);
#else
    // code for different boards here
    DisplayST7789_2bit(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1 = -1, i16 PowerPin2 = -1, i16 BL_pin = -1, i16 RX_pin_dummy = -1, SPIClass *spi = &SPI);
#endif

    void startDrawing(u8 textSize = 2) override; // clears screen + other setup work
    // call these in the main loop before/after drawing
    void endDrawing() override; // makes sure the drawed things are actually displayed


    Adafruit_GFX &getGFX() override;
};
#endif /* SRC_DisplayST7789_FRAMEBUF_2BIT_H_ */