
#include "rp2040Tools.h"
#include "EmbedUI.h"
#include "metric.h"
#include "oscilloscopeUtils.h"
#include "joystickButtons.h"
//#include <pioRecorderDynamic.h>
#include "pioRecorder.h"
//#include <DisplayCommon.h>
#include "DisplayST7789_framebuf_2bit.h"
//#include <DisplaySSD1306.h>


#define DISPLAY_WHITE 3
#define DISPLAY_COLOR1 1
#define DISPLAY_COLOR2 2
#define DISPLAY_BLACK 0

constexpr i32 highestIndexADCclock = 5;
enum ADCclock : i32
{
    clock250kSa = 0,
    clock500kSa = 1,
    clock1MSa = 2,
    clock2MSa = highestIndexADCclock - 2,
    clock2_5MSa = highestIndexADCclock - 1,
    clock3MSa = highestIndexADCclock
};
u32 enumToClk[] = {
    [0] = 40000,//24000,
    [1] = 100000,//48000,
    [2] = 120000,//96000,
    [highestIndexADCclock - 2] = 144000,//96000 * 2,
    [highestIndexADCclock - 1] = 200000,//9600 * 25,
    [highestIndexADCclock] = 240000//96000 * 3
    };

//DisplayST7789_2bit display1(21,20,19,18,17,16,22,20);//breadboard version
DisplayST7789_2bit display1(0,17,3,2,-1,-1,-1,16);//version 1.0 with only ADC chip 0
DisplayCommon &displayMenu = display1;
DisplayCommon &displayGraph = display1;
Adafruit_GFX &gfxMenu = display1.getGFX();
Adafruit_GFX &gfxGraph = display1.getGFX();

//JoystickButtons buts(1,0,2);//breadboard version
JoystickButtons buts(20,21);// version 1.0


/*const String settingADCclockString[] = {"speed:\n"
                                        "160 kSa/s\n"
                                        "250 kSa/s\n"
                                        "500 kSa/s\n"
                                        "  2 MSa/s\n"
                                        "2.5 MSa/s\n"
                                        "  3 MSa/s"};
SettingEnum settingADCclock(settingADCclockString);*/
#define GRAPH_HEIGHT 128
#define GRAPH_WIDTH 240
#define GRAPH_HEIGHT_OFFSET 112

#define DATABUF_SIZE 200000l
volatile u8 data[DATABUF_SIZE];

volatile i32 captureIndex=-1;
bool startScreen = true;
bool onlyPositiveVoltageRange;
bool noInputStage=false;

//bool redraw=true;

ADCclock ADCclockSelection = clock1MSa;

u32 clkWanted;

u32 adcClkDivInfo=2;//96 for internal ADC

void settingsScreenLoop()
{
    displayMenu.startDrawing(1);
    buts.evaluate();
    if(buts.getButtonEvent())
    {
        startScreen=false;
    }
    ADCclockSelection = (ADCclock)(((i32)ADCclockSelection) + buts.getUpEvents());
    if (ADCclockSelection > highestIndexADCclock)
    {
        ADCclockSelection = (ADCclock)0;
    }
    if (ADCclockSelection < 0)
    {
        ADCclockSelection = (ADCclock)highestIndexADCclock;
    }
    gfxMenu.print("speed: ");
    u32 kSa = enumToClk[ADCclockSelection] / adcClkDivInfo;
    gfxMenu.print(kSa);
    gfxMenu.println("kSa/s");

    gfxMenu.println();
    gfxMenu.println("press .");
    gfxMenu.println("  to");
    gfxMenu.println("confirm");
    gfxMenu.println("settings");

    displayMenu.endDrawing();
}
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    display1.Init();
    display1.setRotation(2);
    while(startScreen)
    {
        settingsScreenLoop();
    }
    digitalWrite(LED_BUILTIN,HIGH);
}
u8 highFor2Vref=14;
u8 refSelLowIsInternal=15;
void setup1()//2nd core
{
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);

    pinMode(highFor2Vref,OUTPUT);
    digitalWrite(highFor2Vref,HIGH);

    pinMode(refSelLowIsInternal, OUTPUT);
    digitalWrite(refSelLowIsInternal, LOW);

    while(startScreen)
    {
        delay(1);
    }

    clkWanted = enumToClk[ADCclockSelection];
    if (ADCclockSelection == clock500kSa)
    {
        setCPUClockspeed(133000); // max speed by specs
        delay(100);
    }
    else
    {
        if (clkWanted > 200000)
        {
            setCoreVoltageOffset(4);
            delay(100);
        }
        else if (clkWanted > 180000)
        {
            setCoreVoltageOffset(2);
            delay(100);
        }
        setADCclockBoundToCPUclock(true); // ADC now at adcClock/96000 MSa/s
        setCPUClockspeed(clkWanted);
    }

}

float scaleTime = 1024;
u32 startPos = 0;

void printPIOdebugInfo()
{
    gfxMenu.print("PIO rx FIFO lvl: ");
    gfxMenu.print(pio_sm_get_rx_fifo_level(pio0, 0));
    gfxMenu.print("; TX: ");
    gfxMenu.print(pio_sm_get_tx_fifo_level(pio0, 0));
    gfxMenu.print("; stall: ");
    gfxMenu.println(pio_sm_is_exec_stalled(pio0, 0));
    gfxMenu.print("PIO PC: ");
    for(u32 i=0;i<8;i++)
    {
        gfxMenu.print(pio_sm_get_pc(pio0, 0));
        gfxMenu.print('|');
    }
    gfxMenu.println();
}

float convertVoltage(u16 rawValue,bool onlyPositive=true)
{
    float ret=rawValue;
    if (sizeof(data[0])==1)
    {
        ret /= 256.0f;
    }
    else
    {
        ret /= 4096.0f;
    }
    if(!onlyPositive)
    {
        ret-=0.5f;
    }
    ret *= (3.00f * 11.0f);//3.27V default reference, 11:1 voltage divider
    return ret;
}
bool err=false;
bool unconv=false;
volatile i8 samplesInFIFO = 0;
float tuningFactor=1;
LoopTimer lt;


u16 instr0;
u16 instr1;
u32 instrS;

void loop()
{
    float time=lt.tick();
    volatile u8 *dataToDisplay= data;
    //dataToDisplay[0]^=65536/4;//fake an unconverged sample in 1/2 of the frames
    buts.evaluate();
    i8 upDir=buts.getUpEvents();
    if(upDir>0)
    {
        scaleTime/=2;
        //redraw=true;
    }
    else if(upDir<0)
    {
        scaleTime *= 2;
        //redraw = true;
    }
    i8 rightDir=buts.getRightEvents();
    startPos += scaleTime * 32 * rightDir; // auto-protects against underflow
    if (startPos >= DATABUF_SIZE)
    {
        startPos = DATABUF_SIZE - 1;
    }
    // tuningFactor*=1.01f;
    //if (rightDir!=0)
    //    redraw = true;

    displayMenu.startDrawing(1);

    BandwidthExtenderRC bwExt(((1000000.0f*adcClkDivInfo) / enumToClk[ADCclockSelection]), 26, 10, tuningFactor);//224pF (theoret. 236pF) for P11, 84pF für Bernd, 26pF for P11.2
    u16 voltMax = 0;
    u16 voltMin = 4096;
    for (u32 i = 0; i < DATABUF_SIZE; i++)//find highest + lowest measured voltage
    {
        u16 rawVal = dataToDisplay[i];//get the value from memory only once, as this prevents a previous bug of the value being changed by the DMA in between the statements below
        u16 rawValMasked = rawVal & 4095; //filter out unconverged flag
        if (rawVal != rawValMasked)
        {
            unconv = true;
        }
        if (rawValMasked < voltMin)
        {
            voltMin = rawValMasked;
        }
        if (rawValMasked > voltMax)
        {
            voltMax = rawValMasked;
        }
    }
    float scaleAdd = -float(voltMin);
    float scaleDiv = float(voltMax - voltMin);
    if (scaleDiv < 1)
    {
        scaleDiv = 1; //prevent /0
    }
    GraphDisplaySpace gds(GRAPH_HEIGHT, GRAPH_WIDTH);
    i32 lastIndex = -1;
    float lastVoltage = 0;
    //if(redraw)
    //{
        gfxGraph.fillScreen(ST77XX_BLACK); // Fill screen with black color
        gfxGraph.setCursor(0, 0);          // Set cursor to top-left corner;

        for (i32 i = 0; i < GRAPH_WIDTH; i++)
        {
            i32 index = i * scaleTime + startPos;
            if (index < 0)
            {
                continue;
            }
            if (index >= DATABUF_SIZE)
            {
                continue;
            }
            float voltage = dataToDisplay[index] & 4095; // filter out unconverged flag
            if (index == lastIndex)
            {
                voltage = lastVoltage;
            }
            /*else if (scaleTime<5)
            {
                u32 increments = index - lastIndex;
                i32 bwX = ~(i32(0));
                for (u32 i = 0; i < increments; i++)
                {
                    bwX = bwExt.calcStep(dataToDisplay[lastIndex + i] & 4095);
                }
                if (bwX != ~(i32(0)))
                {
                    voltage = bwX / 65536.0f;
                }
            }*/
            lastVoltage = voltage;
            voltage += scaleAdd;
            voltage /= scaleDiv;
            u16 pixY = gds.toY(voltage);
            if (pixY == -1)
            {
                continue;
            }
            if (dataToDisplay[index] >= 4096)
            {
                gfxGraph.drawPixel(i, pixY - 1, DISPLAY_COLOR1); // mark unconverged sample
                gfxGraph.drawPixel(i, pixY + 1, DISPLAY_COLOR1);
            }
            gfxGraph.drawPixel(i, pixY + GRAPH_HEIGHT_OFFSET, DISPLAY_COLOR2);
        }
        /*if(captureIndex==DATABUF_SIZE)
        {
            redraw=false;
        }
        if (captureIndex <= 0)
        {
            redraw = false;
        }
    }*/

    if(captureIndex==-1)
    {
        gfxMenu.println("Press . for capture");
    }
    else
    {
        gfxMenu.print(captureIndex);
        gfxMenu.print('/');
        gfxMenu.println(DATABUF_SIZE);
    }
    gfxMenu.print("top:    ");
    metricPrintln(convertVoltage(voltMax, onlyPositiveVoltageRange), 'V', gfxMenu, 4);
    gfxMenu.print("bottom: ");
    metricPrintln(convertVoltage(voltMin, onlyPositiveVoltageRange), 'V', gfxMenu, 4);
    gfxMenu.print(startPos);
    gfxMenu.print("; ");
    gfxMenu.print("timespan: ");
    float timePerSample = (adcClkDivInfo* 1000.0f/enumToClk[ADCclockSelection]) / 1000.0f / 1000.0f;
    metricPrintln(timePerSample * scaleTime * GRAPH_WIDTH, 's', gfxMenu, 4);
    if (err)
    {
        gfxMenu.println("error capturing");
    }
    if (unconv)
    {
        gfxMenu.println("unconverged samples");
    }
    gfxMenu.print("raw min: ");
    gfxMenu.println(voltMin);
    gfxMenu.print("raw max: ");
    gfxMenu.println(voltMax);
    gfxMenu.print("tuning: ");
    gfxMenu.println(tuningFactor);
    gfxMenu.print("clk: ");
    gfxMenu.println(getCPUClockspeed() / 1000000.0f);
    printPIOdebugInfo();
    metricPrintln(time, 's', gfxMenu);
    gfxMenu.print(instr0, HEX);
    gfxMenu.print(' ');
    gfxMenu.print(instr1, HEX);
    gfxMenu.print(' ');
    gfxMenu.println(instrS);
    // displayMenu.print("corrMtp: ");
    // displayMenu.println(bwExt.correctionMultiplier);

    // displayGraph.display();
    displayMenu.endDrawing();

}
bool capturePIOcombined(u8 startPin, u8 clkPin)
{
    u8 dataBits = sizeof(data[0]) * 8;
    for (u8 i = startPin; i < startPin + dataBits; i++)
    {
        pinMode(i, INPUT);
    }
    captureIndex = 0;
    PIO pio = pio0;
    PioInstance pioI(pio);
    // pio_clear_instruction_memory(pio);
    DMAinstance dmaI;
    // dmaI.setTransferBitSize(32);
    // dmaI.targetRAMbuffer(data, DATABUF_SIZE/4);
    dmaI.setTransferBitSize(sizeof(data[0]) * 8);
    dmaI.targetRAMbuffer(data, DATABUF_SIZE);
    dmaI.sourceFromPIO(false, 0);
    dmaI.setHighPriority();

    //PioProgram prog
    PioProgram prog(&(recordADC08100_program_instructions[0]), sizeof(recordADC08100_program_instructions) / 2);
    //instr0 = progADC08100.instructions[0];
    //instr1 = progADC08100.instructions[1];
    //instrS = progADC08100.instructions.size();
    pioI.setProgram(prog);


    pioI.sm0.clearFIFOs();
    pioI.sm0.claim();
    //pio_sm_config c = recordADC08100_program_get_default_config(0); // recordADS830E_program_get_default_config(0); // timingTester_program_get_default_config(0);
    // pio_add_program_at_offset(pio, &recordADS830E_program, 0); //&timingTester_program,0);
    // pio_sm_config c = recordADS830E_program_get_default_config(0); // timingTester_program_get_default_config(0);

    //sm_config_set_sideset(&c, 1, false, false);
    //sm_config_set_sideset_pins(&c, clkPin);
    pioI.sm0.setPindirs(clkPin, 1, true);
    pio_gpio_init(pio, clkPin); // TODO
    pioI.sm0.setPindirs(startPin, 8, false);
    //pio_add_program_at_offset(pio, &recordADC08100_program, 0); //&recordADS830E_program, 0) //&timingTester_program,0);
    captureIndex++;
    //pioI.sm0.smConfig = c;
    pioI.sm0.setAutomaticLoopLineNumbers(recordADC08100_wrap_target, recordADC08100_wrap);
    pioI.sm0.setSidesetPins(clkPin,1,false,false);
    pioI.sm0.setIsrPush(false,true,8);
    pioI.sm0.setFirstInPin(startPin);
    pioI.sm0.setClockDivider(1);
    pioI.sm0.setBothFIFOsRX();
    pioI.sm0.setStartInstruction(0);
    //pio_sm_init(pio, 0, 0, &pioI.sm0.smConfig);
    captureIndex++;
    dmaI.start();
    pioI.sm0.startRunning();
    captureIndex++;
    dmaI.waitUntilFinished();
    captureIndex++;
    // delay(10000);
    pioI.sm0.stopRunning();

    pioI.clearInstructionMemory();
    pioI.sm0.unclaim();

    captureIndex = DATABUF_SIZE;
    return true;
}
bool capturePIOold2(u8 startPin,u8 clkPin)
{
    u8 dataBits = sizeof(data[0]) * 8;
    for (u8 i = startPin; i < startPin + dataBits; i++)
    {
        pinMode(i, INPUT);
    }
    captureIndex = 0;
    PIO pio = pio0;
    //pio_clear_instruction_memory(pio);
    DMAinstance dmaI;
    //dmaI.setTransferBitSize(32);
    //dmaI.targetRAMbuffer(data, DATABUF_SIZE/4);
    dmaI.setTransferBitSize(sizeof(data[0]) * 8);
    dmaI.targetRAMbuffer(data, DATABUF_SIZE);
    dmaI.sourceFromPIO(false, 0);
    dmaI.setHighPriority();
    pio_sm_clear_fifos(pio, 0);
    pio_sm_claim(pio,0);
    pio_sm_config c = recordADC08100_program_get_default_config(0); // recordADS830E_program_get_default_config(0); // timingTester_program_get_default_config(0);
    //pio_add_program_at_offset(pio, &recordADS830E_program, 0); //&timingTester_program,0);
    //pio_sm_config c = recordADS830E_program_get_default_config(0); // timingTester_program_get_default_config(0);
    sm_config_set_clkdiv(&c, 1);
    sm_config_set_in_pins(&c, startPin);
    sm_config_set_sideset(&c, 1, false, false);
    sm_config_set_sideset_pins(&c, clkPin);
    //sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    sm_config_set_in_shift(&c, false, true, 8); // ISR
    pio_sm_set_consecutive_pindirs(pio, 0, clkPin, 1, true);
    pio_gpio_init(pio, clkPin); // TODO
    pio_sm_set_consecutive_pindirs(pio, 0, startPin, 8, false);
    pio_add_program_at_offset(pio, &recordADC08100_program, 0); //&recordADS830E_program, 0) //&timingTester_program,0);
    captureIndex++;
    pio_sm_init(pio, 0, 0, &c);
    captureIndex++;
    dmaI.start();
    pio_sm_set_enabled(pio, 0, true);
    captureIndex++;
    dmaI.waitUntilFinished();
    captureIndex++;
    //delay(10000);
    pio_sm_set_enabled(pio, 0, false);

    pio_clear_instruction_memory(pio);
    pio_sm_unclaim(pio, 0);

    captureIndex = DATABUF_SIZE;
    return true;
}

bool capturePIOold(u8 startPin, u8 clkPin)
{
    captureIndex = 0;
    PioInstance pioI(pio0); // PIO pio = pio0;
    DMAinstance dmaI;
    DMAinstance dmaI2;
    // dmaI.setTransferBitSize(32);
    // dmaI.targetRAMbuffer(data, DATABUF_SIZE/4);
    dmaI.setTransferBitSize(sizeof(data[0]) * 8);
    dmaI2.setTransferBitSize(sizeof(data[0]) * 8);
    captureIndex++;
    volatile u32 dummy;
    dmaI.targetRAMbuffer(data, DATABUF_SIZE);
    dmaI2.targetRAMbuffer(&dummy, 4);
    dmaI2.setWriteAddrRing(2);
    captureIndex++;
    dmaI.sourceFromPIO(false, 0);
    dmaI2.sourceFromPIO(false, 0);
    captureIndex++;
    dmaI.setHighPriority();
    dmaI2.setHighPriority();
    // pio_sm_clear_fifos(pio, 0);
    captureIndex++;
    dmaI2.start();
    captureIndex++;
    dmaI2.setChainTo(dmaI);
    pioI.sm0.claim(); // pio_sm_claim(pio,0);
    captureIndex++;
    PioProgram progADC08100(recordADC08100_program_instructions, sizeof(recordADC08100_program_instructions)/2);
    captureIndex++;
    pioI.sm0.setAutomaticLoopLineNumbers(recordADC08100_wrap_target, recordADC08100_wrap); // pioI.sm0.setConfig(recordADC08100_program_get_default_config(0)); // pio_sm_config c = recordADC08100_program_get_default_config(0); // recordADS830E_program_get_default_config(0); // timingTester_program_get_default_config(0);
    captureIndex++;
    // pio_add_program_at_offset(pio, &recordADS830E_program, 0); //&timingTester_program,0);
    // pio_sm_config c = recordADS830E_program_get_default_config(0); // timingTester_program_get_default_config(0);
    pioI.sm0.setFirstInPin(startPin); //(&c, startPin);
    captureIndex++;
    pioI.sm0.setSidesetPins(clkPin, 1, false, false); // sm_config_set_sideset(&c,1,false,false);// sm_config_set_sideset_pins(&c, clkPin);
    captureIndex++;
    pioI.sm0.setClockDivider(1); // sm_config_set_clkdiv(&c, 1);
    captureIndex = 100;
    pioI.sm0.setBothFIFOsRX(); // sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    captureIndex++;
    pioI.sm0.setIsrPush(false, true, 8); // sm_config_set_in_shift(&c, false, true, 8);//ISR
    // pioI.sm0.setOsrPull(false, true, 8);
    captureIndex++;
    pio_gpio_init(pio0, clkPin); // TODO
    captureIndex++;
    pioI.sm0.setPindirs(clkPin, 1, true);
    captureIndex++;
    pioI.sm0.setPindirs(startPin, 8, false);
    captureIndex++;
    pioI.sm0.setStartInstruction(0);
    captureIndex = 200;
    pioI.setProgram(progADC08100); // pio_add_program_at_offset(pio, &ecordADC08100_program, 0);     //&recordADS830E_program, 0); //&timingTester_program,0);
    captureIndex++;
    pioI.sm0.startRunning(); // pio_sm_init(pio, 0, 0, &c); //pio_sm_set_enabled(pio, 0, true);
    captureIndex++;
    pio_sm_set_enabled(pio0, 0, true);
    captureIndex++;
    // captureIndex = 0;
    dmaI2.waitUntilFinished();
    captureIndex++;
    dmaI.waitUntilFinished();
    captureIndex++;
    // captureIndex = 1;
    pioI.sm0.stopRunning(); // pio_sm_set_enabled(pio, 0, false);
    captureIndex++;
    pioI.sm0.unclaim(); // pio_clear_instruction_memory(pio);//pio_sm_unclaim(pio,0);

    captureIndex = DATABUF_SIZE;
    return true;
}
bool capturePIO(u8 startPin,u8 clkPin)
{
    captureIndex = 0;
    u8 dataBits = sizeof(data[0]) * 8;

    //pinMode(clkPin, OUTPUT);
    for (u8 i = startPin; i < startPin + dataBits; i++)
    {
        pinMode(i, INPUT);
    } // TODO
    pio_gpio_init(pio0, clkPin);
    PioInstance pioI(pio0);
    pioI.sm0.setPindirs(clkPin, 1, true);

    DMAinstance dmaI;
    dmaI.targetRAMbuffer(data,DATABUF_SIZE);
    dmaI.setTransferBitSize(dataBits);
    dmaI.sourceFromPIO(false,0);
    dmaI.setHighPriority();
    dmaI.start();

    PioProgram progADC08100(recordADC08100_program_instructions, sizeof(recordADC08100_program_instructions) / 2);
    pioI.setProgram(progADC08100);
    captureIndex = 1;


    pioI.sm0.claim();
    pioI.sm0.setAutomaticLoopLineNumbers(recordADC08100_wrap_target,recordADC08100_wrap);
    pioI.sm0.setBothFIFOsRX();
    pioI.sm0.setClockDivider(1);
    pioI.sm0.setFirstInPin(startPin);
    pioI.sm0.setIsrPush(false, true, dataBits);
    pioI.sm0.setPindirs(startPin, dataBits, false);
    pioI.sm0.setSidesetPins(clkPin,1,false,false);
    pioI.sm0.setStartInstruction(0);
    pioI.sm0.startRunning();
    captureIndex = 2;
    dmaI.waitUntilFinished();
    captureIndex = 3;

    pioI.sm0.stopRunning();
    pioI.sm0.unclaim();
    pioI.clearInstructionMemory();
    captureIndex = DATABUF_SIZE;
    return true;
}
/*
bool captureDMA(u8 pin=ADC1)
{
    unconv=false;
    captureIndex = -5;
    prepareADCforDMA(pin, sizeof(data[0])==1,true);
    captureIndex = -4;
    DMAinstance dmaI;
    dmaI.setTransferBitSize(sizeof(data[0])*8);
    dmaI.targetRAMbuffer(data,DATABUF_SIZE);
    dmaI.sourceFromADCfifo();
    dmaI.setHighPriority();
    dmaI.start();
    captureIndex = -2;
    startFreeRunningMode();
    //delay(1); //Microseconds(20);
    samplesInFIFO = adc_fifo_get_level();
    //dmaI.start();
    captureIndex = 0;
    dmaI.waitUntilFinished();
    captureIndex = 1;
    bool missed = stopFreeRunningMode();
    captureIndex = 3;
    if (missed)
    {
        captureIndex = 8;
        stopFreeRunningMode();
        return false;
    }
    captureIndex = DATABUF_SIZE;
    stopFreeRunningMode();//do not fuse with the one inside the if, it clears flag needed for the if
    return true;
}*/

void loop1()//2nd core
{
    if(buts.getButtonEvent())
    {
        setCPUClockspeed(clkWanted);
        delay(20);
        onlyPositiveVoltageRange = analogRead(A0) < 256;
        //redraw=true;
        err = !capturePIOcombined(4,22);//6,5);//!captureDMA(ADC1); // captureRapid(ADC2);
        setCPUClockspeed(180000);
        //err = !captureRapidMulti(2);aDS
    }
    else
    {
        // digitalWrite(LED_BUILTIN, HIGH);
        // captureIndex=-1;
        delay(10);
        // digitalWrite(LED_BUILTIN, LOW);
        // delay(20);
    }
}