#include "rp2040PWM.h"
#include "hardware/pwm.h"
#include "hardware/regs/pwm.h"

u8 syncPWMs = 0;

void PWMslice::setClockDivider(float div)
{
    pwm_set_clkdiv(sliceNum, div);
}

void PWMslice::setCounterMax(u16 periodMinus1)
{
    pwm_set_wrap(sliceNum,periodMinus1);
}

void PWMslice::setPhaseCorrect(bool phaseCorrect)
{
    pwm_set_phase_correct(sliceNum,phaseCorrect);
}

void PWMslice::setOutputPolarities(bool invertEvenPin, bool invertOddPin)
{
    pwm_set_output_polarity(sliceNum,invertEvenPin,invertOddPin);
}

void PWMslice::setEvenPinInverted(bool inverted)
{
    u32 changemask = 4;// 0100
    u32 keepmask = ~changemask;
    if(!inverted)
    {
        changemask = 0;
    }
    pwm_hw->slice[sliceNum].csr = changemask | (keepmask&(pwm_hw->slice[sliceNum].csr));
}

void PWMslice::setOddPinInverted(bool inverted)
{
    u32 changemask = 8;// 1000
    u32 keepmask = ~changemask;
    if (!inverted)
    {
        changemask = 0;
    }
    pwm_hw->slice[sliceNum].csr = changemask | (keepmask & (pwm_hw->slice[sliceNum].csr));
}

void PWMslice::setPinInverted(u8 pin, bool inverted)
{
    if((pin%2)==0)
    {
        setEvenPinInverted(inverted);
    }
    else
    {
        setOddPinInverted(inverted);
    }
}

void PWMslice::setCounterValue(u16 val)
{
    pwm_set_counter(sliceNum,val);
}

void PWMslice::start()
{
    pwm_set_enabled(sliceNum,true);
}

void PWMslice::freeze()
{
    pwm_set_enabled(sliceNum, false);
}

void PWMslice::stopAt0()
{
    volatile u32 &bothDuty = refToBothDuty();
    u32 prevDuty = bothDuty;
    bothDuty = 0;

    volatile u32 &top = refToCounterMax();

    volatile u32 &rawClkDiv = refToRawClkDivRegister();
    u32 prevClkDiv = rawClkDiv;
    rawClkDiv=0x10;// set to a divider of 1

    volatile u32 &counter = refToRawCounterRegister();
    if(top>4)
    {
        counter = top - 2;//reduce the clock cycles until reaching TOP to reach it in <1µs
    }
    delayMicroseconds(1); // assuming a clockspeed >= 4MHz, this makes sure the counter has reached TOP at least once, updating the pin state to off
    freeze();

    // restore values
    bothDuty = prevDuty;
    rawClkDiv = prevClkDiv;
}

void PWMslice::stopAt1()
{
    volatile u32 &top = refToCounterMax();
    u16 prevTop = top;

    volatile u32 &bothDuty = refToBothDuty();
    u32 prevDuty = bothDuty;
    bothDuty = 0xFFFFFFFF; // set both to maximum values

    volatile u32 &rawClkDiv = refToRawClkDivRegister();
    u32 prevClkDiv = rawClkDiv;
    rawClkDiv = 0x10; // set to a divider of 1 to speed things up

    volatile u32 &counter = refToRawCounterRegister();

    if(top==0xFFFF)//cannot reach 100% duty in this case
    {
        top--;
    }
    if (top > 4)
    {
        counter = top - 2; // reduce the clock cycles until reaching TOP to reach it in <1µs
    }
    delayMicroseconds(1); // assuming a clockspeed >= 4MHz, this makes sure the counter has reached TOP at least once, updating the pin state to off
    freeze();

    // restore values
    bothDuty = prevDuty;
    rawClkDiv = prevClkDiv;
    top=prevTop;
}

void PWMslice::incrementCounter()
{
    pwm_advance_count(sliceNum);
}

void PWMslice::decrementCounter()
{
    pwm_retard_count(sliceNum);
}

void PWMslice::setEvenDuty(u16 val)
{
    pwm_set_chan_level(sliceNum, 0, val);
}

void PWMslice::setOddDuty(u16 val)
{
    pwm_set_chan_level(sliceNum, 1, val);
}

void PWMslice::setPinDuty(u8 pin, u16 val)
{
    pwm_set_gpio_level(pin,val);
}

union rp2040PWMhelperUnion
{
    volatile u32 reg;
    volatile u16 vals[2];
};

volatile u16 &PWMslice::refToEvenPinDuty()
{
    volatile rp2040PWMhelperUnion *helperUnion = (rp2040PWMhelperUnion *)(&(pwm_hw->slice[sliceNum].cc));
    return helperUnion->vals[0];
}

volatile u16 &PWMslice::refToOddPinDuty()
{
    volatile rp2040PWMhelperUnion *helperUnion = (rp2040PWMhelperUnion *)(&(pwm_hw->slice[sliceNum].cc));
    return helperUnion->vals[1];
}

volatile u32 &PWMslice::refToBothDuty()
{
    return pwm_hw->slice[sliceNum].cc;
}

volatile u16 &PWMslice::refToPinDuty(u8 pin)
{
    if((pin%2)==0)
    {
        return refToEvenPinDuty();
    }
    else
    {
        return refToOddPinDuty();
    }
}

volatile u32 &PWMslice::refToRawClkDivRegister()
{
    return pwm_hw->slice[sliceNum].div;
}

volatile u32 &PWMslice::refToRawCounterRegister()
{
    return pwm_hw->slice[sliceNum].ctr;
}

volatile u32 &PWMslice::refToCounterMax()
{
    return pwm_hw->slice[sliceNum].top;
}

void PWMslice::simultaneouslyEnable(bool startAllCalledSlicesNow)
{
    syncPWMs |= (1<<sliceNum);
    if(startAllCalledSlicesNow)
    {
        pwm_set_mask_enabled(syncPWMs);
        syncPWMs = 0;
    }
}

u32 PWMslice::getRawDreqID()
{
    return pwm_get_dreq(sliceNum);
}

PWMslice::PWMslice(u8 pin, bool initPins):
sliceNum(pwm_gpio_to_slice_num(pin))
{
    if(initPins)
    {
        gpio_set_function(pin, GPIO_FUNC_PWM);
    }
}

PWMslice::PWMslice(u8 evenPin, u8 oddPin, bool initPins):
sliceNum(pwm_gpio_to_slice_num(evenPin))
{
    if (initPins)
    {
        gpio_set_function(evenPin, GPIO_FUNC_PWM);
        gpio_set_function(oddPin, GPIO_FUNC_PWM);
    }
}
