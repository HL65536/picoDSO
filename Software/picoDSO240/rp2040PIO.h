#ifndef SRC_RP2040PIO_H_
#define SRC_RP2040PIO_H_

#include "shortNames.h"
#include <vector>

class PioInstructionChange
{
public:
    u16 delta;

    PioInstructionChange(u16 i);

    PioInstructionChange operator*(int mult);
};

class DynamicPioInstruction
{
public:
    u16 instruction;
    DynamicPioInstruction(u16 i);

    //apply a change
    DynamicPioInstruction operator+(PioInstructionChange change);
    void operator+=(PioInstructionChange change);

    //remove a change
    DynamicPioInstruction operator-(PioInstructionChange change);
    void operator-=(PioInstructionChange change);

    //determine difference (= needed change) between instructions
    PioInstructionChange operator-(DynamicPioInstruction other);
};

class PioProgram
{

public:
    std::vector<u16> instructions;

    void addInstruction(u16 instructionCode);
    void addInstruction(DynamicPioInstruction instruction);
    void replaceInstruction(u16 instructionCode, u8 lineNum);
    void replaceInstruction(DynamicPioInstruction instruction, u8 lineNum);

    PioProgram(const u16 *instructions, u8 length);
    PioProgram(u8 dummy);
    PioProgram();
};

class PioSM
{
public:
    PIO instance;
    u8 smNum;
    u8 startPC = 0;
    void applyConfig();
    void init(); // applies config and sets startPC, also clears any previous internal states

public:
    pio_sm_config smConfig;

    void setBothFIFOsTX(); // use only OSR
    void setBothFIFOsRX(); // use only ISR
    void setEqualFIFOs();
    void setIsrPush(bool insertMsbThenShiftRight, bool autopush, u8 threshBits);
    void setOsrPull(bool outputLsbThenShiftRight, bool autopull, u8 threshBits);
    void setOutPins(u8 startPin, u8 pinCount);
    void setSetPins(u8 startPin, u8 pinCount);
    void setFirstInPin(u8 pin);
    void setSidesetPins(u8 firstPin, u8 pinCount, bool isOptional, bool setPinDirectionsInsteadOfValues);
    void setPindirs(u8 firstPinToSet, u8 numberOfPinsToSet, bool setToOutput); // do not use while this SM is running!
    void setClockDivider(float div);                                           // resolution 16bit:8bit
    void setAutomaticLoopLineNumbers(u8 firstLoopedInstruction, u8 lastLoopedInstruction);
    void setJumpConditionPin(u8 pin); // what pin the jump if pin instruction listens to
    void resetConfiguration();
    u8 getCurrentExecutedInstructionLine();
    // TODO implement:
    /*u8 getFifoFilledLevelTX();
    u8 getFifoFilledLevelRX();
    u32 getFromRxFifo(bool blocking = true);
    void executeSingleInstruction(u16 instruction, bool blocking);
    void executeSingleInstruction(DynamicPioInstruction instruction, bool blocking);
    void writeToTxFifo(u32 data,bool blocking = true);*/
    void clearFIFOs();
    void startRunning();  // resets internal states
    void resumeRunning(); // does not reset internal states
    void stopRunning();
    void claim();                                                             // claim ownership of this SM
    void unclaim(bool resetInternals = true, bool resetIfNotClaimed = false); // can safely be called even if not claimed
    bool isClaimed();                                                         // check if somebody has already claimed ownership
    void resetInternalStates(bool resetConfig = false);                       // reset PC, FIFOs, shift counters, ...
    void setStartInstruction(u8 line);

    // void setConfig(pio_sm_config config);// set the config to use (a copy of) the given externally created config

    PioSM(PIO pio, u8 sm);
};

class PioInstance
{
    public:
    PIO self;
    bool isClaimed;
public:
    PioSM sm[4];
    PioSM &sm0;
    PioSM &sm1;
    PioSM &sm2;
    PioSM &sm3;
    void setProgram(PioProgram& program);//replaces any previously placed programs
    void clearInstructionMemory();
    PioInstance(PIO whatPIO, bool claimResources = true);
    ~PioInstance(); // also resets instruction memory (only if claimResources was true)
};

#endif /*SRC_RP2040PIO_H_*/