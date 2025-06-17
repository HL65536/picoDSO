#include "pioRecorderDynamic.h"

DynamicPioInstruction input8(allNeededInstructions_program_instructions[0]);
DynamicPioInstruction input8side1(allNeededInstructions_program_instructions[1]);
DynamicPioInstruction input16(allNeededInstructions_program_instructions[2]);
DynamicPioInstruction input16side1(allNeededInstructions_program_instructions[3]);
DynamicPioInstruction nop(allNeededInstructions_program_instructions[4]);
DynamicPioInstruction nopSide1(allNeededInstructions_program_instructions[5]);
PioInstructionChange clockHigh = input8side1 - input8;

// static_assert((input16side1 - input16).delta == clockHigh.delta);
// static_assert((nopSide1 - nop).delta == clockHigh.delta);

void fillRecordingProgram(PioProgram &p, float readPhase, bool record16bit, u8 instructionsPerClockPhase, bool onlyOneRead)
{
    if (instructionsPerClockPhase == 0)
    {
        return;
    }
    if (instructionsPerClockPhase > 16)
    {
        return; // program would be too large
    }
    DynamicPioInstruction inputInstruction = record16bit ? input16 : input8;
    PioInstructionChange transformToInput = inputInstruction - nop;
    if (onlyOneRead)
    {
        // normalize phase to the range 0-1
        readPhase -= i32(readPhase);
        if (readPhase < 0)
        {
            readPhase++;
        }

        // first create only nop instructions
        for (u8 i = 0; i < instructionsPerClockPhase; i++)
        {
            p.addInstruction(nop);
        }
        for (u8 i = 0; i < instructionsPerClockPhase; i++)
        {
            p.addInstruction(nop + clockHigh);
        }

        // figure out what instruction to change to input
        u8 programLength = instructionsPerClockPhase * 2;
        float clockBasedPhaseShift = 2; // TODO determine exact value empirically
        float indexPhase = programLength * readPhase + clockBasedPhaseShift;
        while (indexPhase >= programLength)
        {
            indexPhase -= programLength;
        }
        while (indexPhase < 0)
        {
            indexPhase += programLength;
        }
        u8 inputIndex = indexPhase;

        // change the correct instruction to input
        p.instructions[inputIndex] += transformToInput.delta;
    }
    else
    {
        for (u8 i = 0; i < instructionsPerClockPhase; i++)
        {
            p.addInstruction(inputInstruction);
        }
        for (u8 i = 0; i < instructionsPerClockPhase; i++)
        {
            p.addInstruction(inputInstruction + clockHigh);
        }
    }
}

void fillRecordingProgram(PioProgram &p, bool record16bit, u8 instructionsPerClockPhase, u8 inputIndex)
{
    if (instructionsPerClockPhase == 0)
    {
        return;
    }
    if (instructionsPerClockPhase > 16)
    {
        return; // program would be too large
    }
    DynamicPioInstruction inputInstruction = record16bit ? input16 : input8;
    PioInstructionChange transformToInput = inputInstruction - nop;

    // first create only nop instructions
    for (u8 i = 0; i < instructionsPerClockPhase; i++)
    {
        p.addInstruction(nop);
    }
    for (u8 i = 0; i < instructionsPerClockPhase; i++)
    {
        p.addInstruction(nop + clockHigh);
    }

    u8 programLength = instructionsPerClockPhase * 2;

    if (inputIndex >= programLength)
    {
        inputIndex = programLength;
    }

    // change the correct instruction to input (from nop)
    p.instructions[inputIndex] += transformToInput.delta;
}
