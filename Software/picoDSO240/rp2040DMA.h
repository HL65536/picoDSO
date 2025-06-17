#ifndef SRC_RP2040DMA_H_
#define SRC_RP2040DMA_H_

#include "shortNames.h"

#include "hardware/dma.h"
#include "rp2040PWM.h"

class DMAinstance
{
    dma_channel_config cConfig;
    u8 channel;

    void init();

public:
    void set8bit();                   // set transfer size to 8 bits per transfer
    void set16bit();                  // set transfer size to 16 bits per transfer
    void set32bit();                  // set transfer size to 32 bits per transfer
    void setTransferBitSize(u8 bits); // allowed: 8,16,32; if forbidden value given, uses 8bit

    void targetRAMbuffer(volatile void *addrStart, u32 numTransfers);
    void targetPWMduty(u8 pin);
    void targetPWMperiodRegister(u8 pin); // register contains [period-1], so if the DMA writes 255, period is 256
    void targetPWMperiodRegister(PWMslice & slice); // register contains [period-1], so if the DMA writes 255, period is 256

    void sourceFromADCfifo();
    void sourceFromRAMbuffer(volatile void *addrStart, u32 numTransfers);
    void sourceRAMvalue(volatile void *addr);     // writes that one value to all of the target
    void sourceFromPIO(bool isPIO1, u8 smNumber); // assumes full 32 bit chunks

    void setTransferAmount(u32 numTransfers); // only needed if both source AND target/destination are single values/peripheral endpoints

    void setEndianessSwap(bool swap = true); // if not called, does not swap

    void setChainTo(DMAinstance &chainTo);

    void setDataPacingSourceRaw(u32 dreqID); // ask autocomplete "DREQ" for options

    void setReadAddrRing(u16 ringSize);  // 0 disables the ring functionality; only powers of 2 allowed 2-32768; can only have EITHER read OR write ring, newer calls override which one from older calls
    void setWriteAddrRing(u16 ringSize); // 0 disables the ring functionality; only powers of 2 allowed 2-32768; can only have EITHER read OR write ring, newer calls override which one from older calls

    void setHighPriority(bool highPriority = true); // if not called, priority is normal

    void waitUntilFinished();
    void abort(); // returns once DMA has stopped; due to errata RP2040-E13 you may see a spurious completion interrupt on the channel as a result of calling this method.
    void start();

    DMAinstance();           // auto-claims a channel
    DMAinstance(u8 Channel); // claims the specified channel
    ~DMAinstance();          // unclaims a channel
};

#endif /*SRC_RP2040DMA_H_*/