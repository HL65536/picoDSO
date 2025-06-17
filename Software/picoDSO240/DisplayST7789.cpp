#include "DisplayST7789.h"


#ifdef ARDUINO_ARCH_RP2040
#include "rp2040Tools.h"
DisplayST7789::DisplayST7789(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1, i16 PowerPin2, i16 BL_pin, i16 RX_pin_dummy, SPIClassRP2040 *spi)
#else
DisplayST7789::DisplayST7789(u8 DC_pin, u8 RST_pin, i16 DA_pin, i16 CL_PIN, i16 PowerPin1, i16 PowerPin2, i16 BL_pin, i16 RX_pin_dummy, SPIClass *spi)
#endif
    : Adafruit_ST7789(spi, -1, DC_pin, RST_pin),
      DisplayCommon(ST7789_WIDTH, ST7789_HEIGHT, false, PowerPin1, PowerPin2),
      DApin(DA_pin), CLpin(CL_PIN), RSTpin(RST_pin), DCpin(DC_pin), BLpin(BL_pin),
      RXpinDummy(RX_pin_dummy), usedSPI(spi)
{}

void DisplayST7789::deinit()
{
    // TODO reset connection needed?
    isInitialized = false;
}

Adafruit_GFX& DisplayST7789::getGFX()
{
    return *this;
}

void DisplayST7789::startDrawing(u8 textSize)
{
    fillRect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, ST77XX_BLACK);
    commonLoopStart(textSize);
}
void DisplayST7789::endDrawing()
{
    delay(100);//placeholder for framebuffer implementation
}

void DisplayST7789::reinit()
{
    init(displayWidth, displayHeight, SPI_MODE3);

    isInitialized = true;
}
void DisplayST7789::shutdown()
{
    deinit();
    if (RSTpin >= 0)
        pinMode(RSTpin, INPUT); // prevent power feed over comm pins
    if (DCpin >= 0)
        pinMode(DCpin, INPUT); // prevent power feed over comm pins
    if (DApin >= 0)
        pinMode(DApin, INPUT); // prevent power feed over comm pins
    if (CLpin >= 0)
        pinMode(CLpin, INPUT); // prevent power feed over comm pins
    if (BLpin >= 0)
        pinMode(BLpin, INPUT); // prevent power feed over comm pins
    switchPowerOff();
}

#include <SPI.h>

void DisplayST7789::resume()
{
    switchPowerOn();
    if (RSTpin != 255)
        pinMode(RSTpin, OUTPUT);
    if (DCpin != 255)
        pinMode(DCpin, OUTPUT);
    if (BLpin != 255)
    {
        pinMode(BLpin, OUTPUT);
        digitalWrite(BLpin,HIGH);
    }


#ifdef ARDUINO_ARCH_RP2040
    usedSPI->setTX(DApin);
    usedSPI->setSCK(CLpin);
    // no MISO, no CS
    usedSPI->setRX(RXpinDummy);//it HAS to use a pin for RX, even if it is not needed for this display
    // the purpose of the dummy is to at least have a choice of what pin will be affected
#endif
    // code for different boards here

    delay(1);
    reinit();
}

void DisplayST7789::Init()
{
    // for different boards, insert code here
#ifdef ARDUINO_ARCH_RP2040
    gpio_set_function(CLpin, GPIO_FUNC_SPI); // clk
    gpio_set_function(DApin, GPIO_FUNC_SPI); // mosi
#endif
    resume();
}
