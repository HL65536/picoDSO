#include "rp2040PIO.h"

#include "hardware/pio.h"

// PioInstructionChange

PioInstructionChange::PioInstructionChange(u16 i):
delta(i)
{}

PioInstructionChange PioInstructionChange::operator*(int mult)
{
    return PioInstructionChange(delta * mult);
}

// DynamicPioInstruction

DynamicPioInstruction::DynamicPioInstruction(u16 i):
instruction(i)
{}

DynamicPioInstruction DynamicPioInstruction::operator+(PioInstructionChange change)
{
    return DynamicPioInstruction(instruction + change.delta);
}

void DynamicPioInstruction::operator+=(PioInstructionChange change)
{
    instruction += change.delta;
}

DynamicPioInstruction DynamicPioInstruction::operator-(PioInstructionChange change)
{
    return DynamicPioInstruction(instruction - change.delta);
}

void DynamicPioInstruction::operator-=(PioInstructionChange change)
{
    instruction -= change.delta;
}

PioInstructionChange DynamicPioInstruction::operator-(DynamicPioInstruction other)
{
    return PioInstructionChange(instruction-other.instruction);
}

// PioProgram

void PioProgram::addInstruction(u16 instructionCode)
{
    instructions.push_back(instructionCode);
}

void PioProgram::addInstruction(DynamicPioInstruction instruction)
{
    instructions.push_back(instruction.instruction);
}

void PioProgram::replaceInstruction(u16 instructionCode, u8 lineNum)
{
    if(lineNum>=instructions.size())
    {
        return;
    }
    if(lineNum>=32)
    {
        return;
    }
    instructions[lineNum]=instructionCode;
}

void PioProgram::replaceInstruction(DynamicPioInstruction instruction, u8 lineNum)
{
    replaceInstruction(instruction.instruction,lineNum);
}

PioProgram::PioProgram(const u16 *instructionsExternal, u8 length)
{
    if (instructionsExternal)
    {
        for (u8 i = 0; i < length; i++)
        {
            instructions.push_back(instructionsExternal[i]);
        }
    }
}

PioProgram::PioProgram(u8 dummy)
{
    //instructions.push_back(42);
}

PioProgram::PioProgram()
{
}

// PioSM

void PioSM::applyConfig()
{
    pio_sm_set_config(instance,smNum,&smConfig);
}

void PioSM::init()
{
    pio_sm_init(instance, smNum, startPC, &smConfig);
}

void PioSM::setBothFIFOsTX()
{
    sm_config_set_fifo_join(&smConfig, PIO_FIFO_JOIN_TX);
}

void PioSM::setBothFIFOsRX()
{
    sm_config_set_fifo_join(&smConfig, PIO_FIFO_JOIN_RX);
}

void PioSM::setEqualFIFOs()
{
    sm_config_set_fifo_join(&smConfig, PIO_FIFO_JOIN_NONE);
}

void PioSM::setIsrPush(bool insertMsbThenShiftRight, bool autopush, u8 pushThreshBits)
{
    sm_config_set_in_shift(&smConfig,insertMsbThenShiftRight,autopush,pushThreshBits);
}

void PioSM::setOsrPull(bool outputLsbThenShiftRight, bool autopull, u8 pullThreshBits)
{
    sm_config_set_out_shift(&smConfig, outputLsbThenShiftRight, autopull, pullThreshBits);
}

void PioSM::setOutPins(u8 startPin, u8 pinCount)
{
    sm_config_set_out_pins(&smConfig, startPin, pinCount);
}

void PioSM::setSetPins(u8 startPin, u8 pinCount)
{
    sm_config_set_set_pins(&smConfig, startPin, pinCount);
}

void PioSM::setFirstInPin(u8 pin)
{
    sm_config_set_in_pins(&smConfig, pin);
}

void PioSM::setSidesetPins(u8 firstPin, u8 pinCount, bool isOptional, bool setPinDirectionsInsteadOfValues)
{
    u8 bits = pinCount;
    if (isOptional)
    {
        bits++;
    }
    sm_config_set_sideset_pins(&smConfig, firstPin);
    sm_config_set_sideset(&smConfig, bits, isOptional, setPinDirectionsInsteadOfValues);
}

void PioSM::setPindirs(u8 firstPinToSet, u8 numberOfPinsToSet, bool setToOutput)
{
    pio_sm_set_consecutive_pindirs(instance,smNum,firstPinToSet,numberOfPinsToSet,setToOutput);
}

void PioSM::setClockDivider(float div)
{
    sm_config_set_clkdiv(&smConfig, div);
}

void PioSM::setAutomaticLoopLineNumbers(u8 firstLoopedInstruction, u8 lastLoopedInstruction)
{
    sm_config_set_wrap(&smConfig, firstLoopedInstruction, lastLoopedInstruction);
}

void PioSM::setJumpConditionPin(u8 pin)
{
    sm_config_set_jmp_pin(&smConfig, pin);
}

void PioSM::resetConfiguration()
{
    smConfig=pio_get_default_sm_config();
    applyConfig();
}

u8 PioSM::getCurrentExecutedInstructionLine()
{
    return pio_sm_get_pc(instance, smNum);
}

void PioSM::clearFIFOs()
{
    pio_sm_clear_fifos(instance,smNum);
}

void PioSM::startRunning()
{
    init();
    resumeRunning();
}

void PioSM::resumeRunning()
{
    pio_sm_set_enabled(instance, smNum, true);
}

void PioSM::stopRunning()
{
    pio_sm_set_enabled(instance, smNum, false);
}

void PioSM::claim()
{
    pio_sm_claim(instance,smNum);
}

void PioSM::unclaim(bool resetInternals, bool resetIfNotClaimed)
{
    if(isClaimed())
    {
        resetInternalStates(true);
        pio_sm_unclaim(instance, smNum);
    }
    else if(resetIfNotClaimed)
    {
        resetInternalStates(true);
    }
}

bool PioSM::isClaimed()
{
    return pio_sm_is_claimed(instance,smNum);
}

void PioSM::resetInternalStates(bool resetConfig)
{
    if(resetConfig)
    {
        resetConfiguration();
    }
    init();
}

void PioSM::setStartInstruction(u8 line)
{
    startPC = line;
}

/*void PioSM::setConfig(pio_sm_config config)
{
    smConfig = config;
}*/

PioSM::PioSM(const PIO pio, u8 sm):
instance(pio),smNum(sm)
{
    smConfig = pio_get_default_sm_config();
}

// PioInstance

void PioInstance::setProgram(PioProgram& program)
{
    clearInstructionMemory();
    //u16 progTemp[32];
    //for (u8 i = 0; i < program.instructions.size(); i++)
    //{
        //progTemp[i]=program.instructions[i];
    //}

    pio_program_t program_struct = {
        .instructions = &(program.instructions[0]),
        .length = program.instructions.size(),
        .origin = -1,
    };
    pio_add_program_at_offset(self, &program_struct, 0);
}

void PioInstance::clearInstructionMemory()
{
    pio_clear_instruction_memory(self);
}

PioInstance::PioInstance(const PIO whatPIO, bool claimResources):
self(whatPIO), isClaimed(claimResources),
sm{{whatPIO, 0}, {whatPIO, 1}, {whatPIO, 2}, {whatPIO, 3}},
sm0(sm[0]), sm1(sm[1]), sm2(sm[2]), sm3(sm[3])
{
}

PioInstance::~PioInstance()
{
    if(isClaimed)//if false, do nothing, as this object does not have ownership of the resources
    {
        for (u8 i = 0; i < 4; i++)
        {
            sm[i].resetInternalStates(true);
            sm[i].unclaim();
        }
        clearInstructionMemory();
    }
}

