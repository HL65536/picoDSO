#include "DisplayST7789_framebuf_2bit.h"
u16 colorLookup[] = {ST77XX_BLACK, ST77XX_GREEN, ST77XX_MAGENTA, ST77XX_WHITE};
#ifdef ARDUINO_ARCH_RP2040
#include "rp2040Tools.h"
DisplayST7789_2bit::DisplayST7789_2bit(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1, i16 PowerPin2, i16 BL_pin, i16 RX_pin_dummy, SPIClassRP2040 *spi)
#else
DisplayST7789_2bit::DisplayST7789_2bit(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1, i16 PowerPin2, i16 BL_pin, i16 RX_pin_dummy, SPIClass *spi)
#endif
: DisplayST7789(DC_pin, RST_pin, DA_pin, CL_PIN, PowerPin1, PowerPin2, BL_pin, RX_pin_dummy, spi)
{
}

Adafruit_GFX &DisplayST7789_2bit::getGFX()
{
    return canvas;
}

void DisplayST7789_2bit::startDrawing(u8 textSize)
{
    memset(canvas.framebuf, 0, sizeof(VirtualCanvasST7789_2bit ::framebuf));

    commonLoopStart(textSize);
}

void DisplayST7789_2bit::endDrawing()
{

    startWrite();
    setAddrWindow(0, 0, ST7789_WIDTH, ST7789_HEIGHT);

    for (u32 i = 0; i < sizeof(canvas.framebuf) / sizeof(canvas.framebuf[0]); i++)
    {
        u32 colorChunk=canvas.framebuf[i];

        for (size_t i = 0; i < 16; i++)
        {
            SPI_WRITE16(colorLookup[colorChunk % 4]);
            colorChunk /= 4;
        }
    }
    endWrite();

}

void VirtualCanvasST7789_2bit::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    u32 colorFiltered = color%4;//filter out wrong parameters in order to not mess with other pixels
    u32 index = (ST7789_WIDTH / 16) * y + x / 16;
    u32 shift = (x % 16) * 2;
    u32 original = framebuf[index];
    u32 mask = ~(3 << shift);
    framebuf[index] = (original & mask) | (colorFiltered << shift);
}

VirtualCanvasST7789_2bit::VirtualCanvasST7789_2bit() : Adafruit_GFX(ST7789_WIDTH, ST7789_HEIGHT)
{}