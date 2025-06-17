#include "DisplayST7789_framebuf_3bit.h"
u16 colorLookup3bit[] = {ST77XX_BLACK, ST77XX_BLUE, ST77XX_GREEN, ST77XX_CYAN, ST77XX_RED, ST77XX_MAGENTA, ST77XX_YELLOW, ST77XX_WHITE};
#ifdef ARDUINO_ARCH_RP2040
#include "rp2040Tools.h"
DisplayST7789_3bit::DisplayST7789_3bit(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1, i16 PowerPin2, i16 BL_pin, i16 RX_pin_dummy, SPIClassRP2040 *spi)
#else
DisplayST7789_3bit::DisplayST7789_3bit(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1, i16 PowerPin2, i16 BL_pin, i16 RX_pin_dummy, SPIClass *spi)
#endif
: DisplayST7789(DC_pin, RST_pin, DA_pin, CL_PIN, PowerPin1, PowerPin2, BL_pin, RX_pin_dummy, spi)
{
}

Adafruit_GFX &DisplayST7789_3bit::getGFX()
{
    return canvas;
}

void DisplayST7789_3bit::startDrawing(u8 textSize)
{
    memset(canvas.framebuf, 0, sizeof(VirtualCanvasST7789_3bit ::framebuf));

    commonLoopStart(textSize);
}

void DisplayST7789_3bit::endDrawing()
{
    startWrite();
    setAddrWindow(0, 0, ST7789_WIDTH, ST7789_HEIGHT);

    for (u32 i = 0; i < sizeof(canvas.framebuf) / sizeof(canvas.framebuf[0]); i++)
    {
        u32 colorChunk=canvas.framebuf[i];

        for (size_t i = 0; i < 10; i++)
        {
            SPI_WRITE16(colorLookup3bit[colorChunk % 8]);
            colorChunk /= 4;
        }
    }
    endWrite();
}

void VirtualCanvasST7789_3bit::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    u32 colorFiltered = color%8;//filter out wrong parameters in order to not mess with other pixels
    u32 index = (ST7789_WIDTH / 10) * y + x / 10;
    u32 shift = (x % 10) * 2;
    u32 original = framebuf[index];
    u32 mask = ~(7 << shift);
    framebuf[index] = (original & mask) | (colorFiltered << shift);
}

VirtualCanvasST7789_3bit::VirtualCanvasST7789_3bit() : Adafruit_GFX(ST7789_WIDTH, ST7789_HEIGHT)
{}