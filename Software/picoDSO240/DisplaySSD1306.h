#ifndef SRC_DISPLAYSSD1306_H_
#define SRC_DISPLAYSSD1306_H_

#include "DisplayCommon.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "shortNames.h"

#define SSD1306_WIDTH 128 // OLED display width, in pixels
#define SSD1306_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
#define SSD1306_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SSD1306_ADDRESS 0x3C ///< See datasheet for Address; commonly 0x3D for 128x64, 0x3C for 128x32
// this #define is now just providing the default parameter for the address, others are therefore possible

// TODO https://www.youtube.com/watch?v=HC5GtOXJFfs
// Änderung der Helligkeit

class DisplaySSD1306 : public Adafruit_SSD1306, public DisplayCommon
{
    // intended as replacement for displayStuff on 32bit-MCs
    // supports multiple displays on one or multiple I2Cs
    // needs a bit more RAM than the simpler variant
protected:
    void deinit() override; // init can then be called again without causing problems
    void reinit() override; // init but shorter and with reused parameters
public:
    bool isSecondaryOnBus = true;
    bool preventCodeContinuationIfInitFails = false;
    TwoWire *TWI;
    i16 SDApin = -1;//-1 means default pin of the board
    i16 SCLpin = -1;
    // pins on Raspberry Pi Pico:
    // 0 = I2C0 SDA; Wire
    // 1 = I2C0 SCL; Wire
    // 2 = I2C1 SDA; Wire1
    // 3 = I2C1 SCL; Wire1
    // repeat %4 (4 = I2C0 SDA, ...)

    u8 addrI2C = 0x3C;

    DisplaySSD1306(TwoWire *twi = &Wire, i16 sdaPin = -1, i16 sclPin = -1,
                   bool PreventCodeContinuationIfInitFails = false,
                   u8 I2CAddr = SSD1306_ADDRESS, bool isSecondaryDisplayOnBus = false,
                   i16 PowerPin = -1, i16 GNDpin = -1,
                   u16 Width = SSD1306_WIDTH, u16 Height = SSD1306_HEIGHT);

    // overclock levels:
    // 8 = recommended for ATMEGA328P; above untested
    // 16 = recommended for RP2040; 20 seems stable; 33 max working semi-stable
    // overclock performance (at recommended levels):
    // ATMEGA328P: 37.2ms vs 26.8ms non-overclock/overclock
    // RP2040: 28.4ms vs 9ms full loop; 28.05ms vs 8.6ms display.display only
    DisplaySSD1306(u8 overclockLevel, TwoWire *twi = &Wire, i16 sdaPin = -1,
                   i16 sclPin = -1, bool PreventCodeContinuationIfInitFails = false,
                   u8 I2CAddr = SSD1306_ADDRESS, bool isSecondaryDisplayOnBus = false,
                   i16 PowerPin = -1, i16 GNDpin = -1,
                   u16 Width = SSD1306_WIDTH, u16 Height = SSD1306_HEIGHT);

    void startDrawing(u8 textSize = 2) override; // clears screen + other setup work
    // call these in the main loop before/after drawing
    void endDrawing() override; // makes sure the drawed things are actually displayed

    void shutdown() override; // cut power to the display and disconnect communication (if possible)
    void resume() override;   // restore power and communication
    void Init() override;     // call this at the beginning of the application before anything else display-related

    Adafruit_GFX &getGFX() override;
};
#endif /* SRC_DISPLAYSSD1306_H_ */