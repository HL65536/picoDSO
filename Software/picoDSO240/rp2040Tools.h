#ifndef SRC_RP2040TOOLS_H_
#define SRC_RP2040TOOLS_H_

#include <Arduino.h>


// MOST OF THE STUFF IN HERE IS NOT IDIOT-PROOF
// if you want idiot proof stuff, use the Arduino standard library

#include "rp2040ADC.h"
#include "rp2040PWM.h"
#include "rp2040PIO.h"
#include "rp2040DMA.h"
#include "rp2040GPIO.h"
#include "rp2040clock.h"
#include "rp2040DAC.h"

/*  TODO missing features:
    - size_t getStackSize()
    - u8 getADCmeasureTarget()
    - void printRAMusage(Print &printTarget, u16 pixelSizeBytes = 1, u16 lineSize = 64) // only works with !=0, so empty display means lots of 0
    - EEprom emulation/flash storage
    - float readTemperature(u16 samples=1) // °C
    - void enterIdleMode() //resumes after first interrupt
    - PWM wrapper
    - PIO wrapper
*/


//how to memory-align (e.g. for DMA ring buffers):
// u32 data[512] __attribute__((aligned(2048)));

void setCoreVoltageOffset(i8 offset);//-5 to +4; default: 0; stepSize: 50mV



#endif /* SRC_RP2040TOOLS_H_ */