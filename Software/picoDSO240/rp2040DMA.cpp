#include "rp2040DMA.h"

#include "hardware/adc.h"

void DMAinstance::set8bit()
{
    channel_config_set_transfer_data_size(&cConfig, DMA_SIZE_8);
}

void DMAinstance::set16bit()
{
    channel_config_set_transfer_data_size(&cConfig, DMA_SIZE_16);
}

void DMAinstance::set32bit()
{
    channel_config_set_transfer_data_size(&cConfig, DMA_SIZE_32);
}

void DMAinstance::setTransferBitSize(u8 bits)
{
    switch (bits)
    {
    case 16:
        set16bit();
        break;
    case 32:
        set32bit();
        break;
    default: // "case 8:" or "case userIsStupid:"
        set8bit();
    }
}
void DMAinstance::targetRAMbuffer(volatile void *addrStart, u32 numTransfers)
{
    dma_channel_set_write_addr(channel, addrStart, false);
    channel_config_set_write_increment(&cConfig, true);
    dma_channel_set_trans_count(channel, numTransfers, false);
}

void DMAinstance::targetPWMduty(u8 pin)
{
    PWMslice p(pin,false);
    dma_channel_set_write_addr(channel, &p.refToPinDuty(pin), false);
    setDataPacingSourceRaw(p.getRawDreqID());
}
void DMAinstance::targetPWMperiodRegister(u8 pin)
{
    PWMslice p(pin, false);
    dma_channel_set_write_addr(channel, &p.refToCounterMax(), false);
    setDataPacingSourceRaw(p.getRawDreqID());
}
void DMAinstance::targetPWMperiodRegister(PWMslice &slice)
{
    dma_channel_set_write_addr(channel, &slice.refToCounterMax(), false);
    setDataPacingSourceRaw(slice.getRawDreqID());
}

void DMAinstance::sourceFromADCfifo()
{
    channel_config_set_dreq(&cConfig, DREQ_ADC);
    channel_config_set_read_increment(&cConfig, false);
    dma_channel_set_read_addr(channel, &adc_hw->fifo, false);
}

void DMAinstance::sourceFromRAMbuffer(volatile void *addrStart, u32 numTransfers)
{
    channel_config_set_read_increment(&cConfig, true);
    dma_channel_set_read_addr(channel, addrStart, false);
    dma_channel_set_trans_count(channel, numTransfers, false);
    channel_config_set_dreq(&cConfig, DREQ_FORCE);
}

void DMAinstance::sourceRAMvalue(volatile void *addr)
{
    channel_config_set_read_increment(&cConfig, false);
    dma_channel_set_read_addr(channel, addr, false);
    channel_config_set_dreq(&cConfig, DREQ_FORCE);
}

void DMAinstance::sourceFromPIO(bool isPIO1, u8 smNumber)
{
    channel_config_set_read_increment(&cConfig, false);

    if (!isPIO1)
    {
        dma_channel_set_read_addr(channel, &(pio0_hw->rxf[smNumber]), false);
        channel_config_set_dreq(&cConfig, pio_get_dreq(pio0, smNumber, false));
    }
    else
    {
        dma_channel_set_read_addr(channel, &(pio1_hw->rxf[smNumber]), false);
        channel_config_set_dreq(&cConfig, pio_get_dreq(pio1, smNumber, false));
    }
}

void DMAinstance::setTransferAmount(u32 numTransfers)
{
    dma_channel_set_trans_count(channel, numTransfers, false);
}

void DMAinstance::setEndianessSwap(bool swap)
{
    channel_config_set_bswap(&cConfig, swap);
}

void DMAinstance::setChainTo(DMAinstance &chainTo)
{
    channel_config_set_chain_to(&cConfig, chainTo.channel);
}

void DMAinstance::setDataPacingSourceRaw(u32 dreqID)
{
    channel_config_set_dreq(&cConfig, dreqID);
}

u8 getSizeBits(u16 ringSize)
{
    switch (ringSize)
    {
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    case 16:
        return 4;
    case 32:
        return 5;
    case 64:
        return 6;
    case 128:
        return 7;
    case 256:
        return 8;
    case 512:
        return 9;
    case 1024:
        return 10;
    case 2048:
        return 11;
    case 4096:
        return 12;
    case 8192:
        return 13;
    case 16384:
        return 14;
    case 32768:
        return 15;
    default:
        return 0;
    }
}

void DMAinstance::setReadAddrRing(u16 ringSize)
{
    channel_config_set_ring(&cConfig, false, getSizeBits(ringSize));
}

void DMAinstance::setWriteAddrRing(u16 ringSize)
{
    channel_config_set_ring(&cConfig, true, getSizeBits(ringSize));
}

void DMAinstance::setHighPriority(bool highPriority)
{
    channel_config_set_high_priority(&cConfig, highPriority);
}

void DMAinstance::abort()
{
    dma_channel_abort(channel);
}

void DMAinstance::start()
{
    dma_channel_set_config(channel, &cConfig, true);
}

void DMAinstance::waitUntilFinished()
{
    dma_channel_wait_for_finish_blocking(channel);
}

void DMAinstance::init()
{
    cConfig = dma_channel_get_default_config(channel);
}

DMAinstance::DMAinstance()
{
    channel = dma_claim_unused_channel(true);
    init();
}

DMAinstance::DMAinstance(u8 Channel) : channel(Channel)
{
    init();
}

DMAinstance::~DMAinstance()
{
    dma_channel_unclaim(channel);
}
