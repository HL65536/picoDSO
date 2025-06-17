
#include "rp2040Tools.h"
#include "EmbedUI.h"
#include "metric.h"
#include "oscilloscopeUtils.h"
#include "joystickButtons.h"
#include "pioRecorderDynamic.h"
#include "DisplayST7789_framebuf_2bit.h"


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
u32 clkSelectionsExt[] = {
    [0] = 40000,//24000,
    [1] = 80000,//48000,
    [2] = 120000,//96000,
    [highestIndexADCclock - 2] = 160000,//96000 * 2,
    [highestIndexADCclock - 1] = 200000,//9600 * 25,
    [highestIndexADCclock] = 240000//96000 * 3
    };

u32 clkSelectionsInt[] = {
    [0] = 24000,
    [1] = 48000,
    [2] = 96000,
    [highestIndexADCclock - 2] = 96000 * 2,
    [highestIndexADCclock - 1] = 9600 * 25,
    [highestIndexADCclock] = 96000 * 3
    };
//DisplayST7789_2bit display1(21,20,19,18,17,16,22,20);//breadboard version
DisplayST7789_2bit display1(0,17,3,2,-1,-1,-1,16);//PCB version 1.0 with only ADC chip 0
DisplayCommon &displayMenu = display1;
DisplayCommon &displayGraph = display1;
Adafruit_GFX &gfxMenu = display1.getGFX();
Adafruit_GFX &gfxGraph = display1.getGFX();

//JoystickButtons buts(1,0,2);//breadboard version
JoystickButtons buts(20,21);//PCB version 1.0


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

Recording recording;//for now only one single recording is saved at a time, may be changed in the future
volatile bool useExternal=true;
volatile i32 captureIndex=-1;

volatile u8 channelCount=1;

bool startScreen = true;
bool onlyPositiveVoltageRange;
bool noInputStage=false;

u16 channelColors[]=//size must be at least equal to channelCount
{
    DISPLAY_COLOR1,
    DISPLAY_COLOR2
};

ADCclock ADCclockSelection = clock1MSa;

u32 clkWanted;

u32 adcClkDivInfo=4;//96 for internal ADC, auto-set later!

void settingsScreenLoop()
{
    displayMenu.startDrawing(1);
    buts.evaluate();
    if(buts.getButtonEvent())
    {
        recording.allocateSpace(DATABUF_SIZE,channelCount);
        startScreen=false;
    }
    else
    {
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
        adcClkDivInfo = useExternal?4:96;
        u32 kSa = clkSelectionsExt[ADCclockSelection] / adcClkDivInfo;
        gfxMenu.print(kSa);
        gfxMenu.println("kSa/s");

        gfxMenu.println();
        gfxMenu.println("press .");
        gfxMenu.println("  to");
        gfxMenu.println("confirm");
        gfxMenu.println("settings");

        displayMenu.endDrawing();
    }
    
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
}
u8 highFor2Vref=14;
u8 refSelLowIsInternal=15;
void setup1()//2nd core
{
    pinMode(LED_BUILTIN, OUTPUT);

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

    clkWanted = clkSelectionsExt[ADCclockSelection];
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
u32 discardSamples = 64;// don't show the first 64 samples to avoid showing ADC startup glitches
u32 startPos = discardSamples;

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
    if (recording.is16bit())
    {
        ret /= 4096.0f;//only supported ADC with 16 bit data length is currently the internal 12-bit ADC
    }
    else
    {
        ret /= 256.0f;
    }
    if(!onlyPositive)
    {
        ret-=0.5f;
    }
    ret *= 15;//(3.00f * 11.0f);//3.27V default reference, 11:1 voltage divider
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
    
    u32 dataLength = recording.getNumSamples()-discardSamples;
    if(recording.getNumSamples()<discardSamples+1)
    {
        delay(1);
        return;
    }
    //dataToDisplay[0]^=65536/4;//fake an unconverged sample in 1/2 of the frames
    buts.evaluate();
    i8 upDir=buts.getUpEvents();
    if(upDir>0)
    {
        scaleTime/=2;
    }
    else if(upDir<0)
    {
        scaleTime *= 2;
    }
    i8 rightDir=buts.getRightEvents();
    startPos += scaleTime * 32 * rightDir; // auto-protects against underflow
    if (startPos >= dataLength)
    {
        startPos = dataLength - 1;
    }
    if (startPos < discardSamples)
    {
        startPos = discardSamples;
    }

    displayMenu.startDrawing(1);

    BandwidthExtenderRC bwExt(((1000000.0f*adcClkDivInfo) / clkSelectionsExt[ADCclockSelection]), 26, 10, tuningFactor);//224pF (theoret. 236pF) for P11, 84pF für Bernd, 26pF for P11.2
    u16 voltMax = 0;
    u16 voltMin = 4096;
    for (u32 i = 0; i < dataLength; i++) // find highest + lowest measured voltage
    {
        u16 rawVal = recording.readIndex(i);//get the value from memory only once, as this prevents a previous bug of the value being changed by the DMA in between the statements below
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
    //i32 lastIndex = -1;
    gfxGraph.fillScreen(ST77XX_BLACK); // Fill screen with black color
    gfxGraph.setCursor(0, 0);          // Set cursor to top-left corner;

    for (i32 i = 0; i < GRAPH_WIDTH; i++)
    {
        i32 index = i * scaleTime + startPos;
        if (index < 0)
        {
            continue;
        }
        if (index >= dataLength)
        {
            continue;
        }
        for (u8 ch = 0; ch < channelCount; ch++)
        {
            u16 rawVal = recording.readIndex(index,ch);
            float voltage = rawVal & 4095; // filter out unconverged flag
                /*if (index == lastIndex)
                {
                    voltage = lastVoltage;
                }
                else if (scaleTime<5)
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
                }
                lastVoltage = voltage;*/
            voltage += scaleAdd;
            voltage /= scaleDiv;
            u16 pixY = gds.toY(voltage);
            //u16 color = channelColors[ch];
            if (pixY == -1)
            {
                continue;
            }
            if (rawVal >= 4096)
            {
                gfxGraph.drawPixel(i, pixY - 1, DISPLAY_COLOR1); // mark unconverged sample
                gfxGraph.drawPixel(i, pixY + 1, DISPLAY_COLOR1);
            }
            gfxGraph.drawPixel(i, pixY + GRAPH_HEIGHT_OFFSET, DISPLAY_COLOR2);
        }
    }
        
    if (captureIndex == -1)
    {
        gfxMenu.println("Press . for capture");
    }
    else
    {
        gfxMenu.print(captureIndex);
        gfxMenu.print('/');
        gfxMenu.println(recording.is16bit()? DATABUF_SIZE/2:DATABUF_SIZE);
    }
    gfxMenu.print("top:    ");
    metricPrintln(convertVoltage(voltMax, onlyPositiveVoltageRange), 'V', gfxMenu, 4);
    gfxMenu.print("bottom: ");
    metricPrintln(convertVoltage(voltMin, onlyPositiveVoltageRange), 'V', gfxMenu, 4);
    gfxMenu.print(startPos);
    gfxMenu.print("; ");
    gfxMenu.print("timespan: ");
    float timePerSample = (adcClkDivInfo* 1000.0f/clkSelectionsExt[ADCclockSelection]) / 1000.0f / 1000.0f;
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
    digitalWrite(LED_BUILTIN,HIGH);
    u8 dataBits = 8*channelCount;
    for (u8 i = startPin; i < startPin + dataBits; i++)
    {
        pinMode(i, INPUT);
    }
    captureIndex = 0;
    PIO pio = pio0;
    PioInstance pioI(pio);
    
    DMAinstance dmaI;
    dmaI.setTransferBitSize(channelCount * 8);
    dmaI.targetRAMbuffer(recording.is16bit()?(void *)(recording.dataPtr16):(void *)(recording.dataPtr8), DATABUF_SIZE);
    dmaI.sourceFromPIO(false, 0);
    dmaI.setHighPriority();

    PioProgram prog;
    fillRecordingProgram(prog,false,2,2);
    pioI.setProgram(prog);

    pioI.sm0.clearFIFOs();
    pioI.sm0.claim();
    
    pioI.sm0.setPindirs(clkPin, 1, true);
    pio_gpio_init(pio, clkPin); // TODO ?
    pioI.sm0.setPindirs(startPin, 8, false);
    
    captureIndex++;
    
    pioI.sm0.setAutomaticLoopLineNumbers(0,prog.instructions.size()-1);
    pioI.sm0.setSidesetPins(clkPin,1,false,false);
    pioI.sm0.setIsrPush(false,true,8);
    pioI.sm0.setFirstInPin(startPin);
    pioI.sm0.setClockDivider(1);
    pioI.sm0.setBothFIFOsRX();
    pioI.sm0.setStartInstruction(0);

    captureIndex++;
    dmaI.start();

    pioI.sm0.startRunning();


    captureIndex++;
    dmaI.waitUntilFinished();
    captureIndex++;

    pioI.sm0.stopRunning();

    pioI.clearInstructionMemory();
    pioI.sm0.unclaim();

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
        onlyPositiveVoltageRange = true;//analogRead(A0) < 256;
        //PCB version: pin 4 is LSB for external ADC, pin 22 is clk for external ADC
        err = !capturePIOcombined(4,22);//6,5);//!captureDMA(ADC1); // captureRapid(ADC2);
        setCPUClockspeed(200000);
        //err = !captureRapidMulti(2);aDS
    }
    else
    {
        delay(10);
    }
}

