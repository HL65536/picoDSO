// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif
#include "shortNames.h"
#include "rp2040Tools.h"
// ------------ //
// allNeededInstructions //
// ------------ //

#define allNeededInstructions_wrap_target 0
#define allNeededInstructions_wrap 5

static const uint16_t allNeededInstructions_program_instructions[] = {
    //     .wrap_target
    0x4008, //  0: in     pins, 8         side 0
    0x5008, //  1: in     pins, 8         side 1
    0x4010, //  2: in     pins, 16        side 0
    0x5010, //  3: in     pins, 16        side 1
    0xa042, //  4: nop                    side 0
    0xb042, //  5: nop                    side 1
            //     .wrap
};

extern DynamicPioInstruction input8;
extern DynamicPioInstruction input8side1;
extern DynamicPioInstruction input16;
extern DynamicPioInstruction input16side1;
extern DynamicPioInstruction nop;
extern DynamicPioInstruction nopSide1;
extern PioInstructionChange clockHigh;

void fillRecordingProgram(PioProgram &p, float readPhase, bool record16bit=false,u8 instructionsPerClockPhase = 1,bool onlyOneRead=true);
void fillRecordingProgram(PioProgram &p, bool record16bit = false, u8 instructionsPerClockPhase = 1, u8 inputIndex = 0);

/*#if !PICO_NO_HARDWARE
    static const struct pio_program allNeededInstructions_program = {
        .instructions = allNeededInstructions_program_instructions,
        .length = 6,
        .origin = -1,
};

static inline pio_sm_config allNeededInstructions_program_get_default_config(uint offset)
{
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + allNeededInstructions_wrap_target, offset + allNeededInstructions_wrap);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}
#endif*/
