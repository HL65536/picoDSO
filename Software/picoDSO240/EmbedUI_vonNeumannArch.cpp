#include "EmbedUI.h"
#include "EmbedUI_vonNeumannArch.h"

//TODO port to von Neumann architecture

// Setting
void Setting::tick(SelectButtons &buts, TimeLight timeElapsed, bool wasReset)
{
    buts.tick(timeElapsed, sel, wasReset);
}

//u16 Setting::storeEEprom(u16 startIndex)
//{
//    u16 state = sel.state;
//    EEPROM.update(startIndex, (u8)state);
//    EEPROM.update(startIndex + 1, state / 256);
//    return startIndex + 2;
//}
//u16 Setting::loadEEprom(u16 startIndex)
//{
//    u16 newState = EEPROM.read(startIndex);
//    newState += 256 * EEPROM.read(startIndex + 1);
//    sel.state = newState;
//    return startIndex + 2;
//}

void Setting::displayString(Print &printTarget)
{
    printTarget.print(*name);
    printTarget.print(' ');
    printTarget.print(sel.getState());
}

u16 Setting::getState()
{
    return sel.getState();
}
u16 Setting::getMax()
{
    return sel.getMax();
}
void Setting::setState(u16 newState)
{
    sel.state = newState;
}
void Setting::setMax(u16 newMax)
{
    sel.changeMax(newMax);
}

Setting::Setting(Selection Sel, const String *const Name) : sel(Sel), name(Name)
{
}

// SettingInt

SettingInt::SettingInt(u16 max, const String *const Name) : Setting(Selection(max), Name)
{
}

// SettingOnOff
void SettingOnOff::displayString(Print &printTarget)
{
    printTarget.print(*name);
    printTarget.print(' ');
    if (sel.getState())
    {
        printTarget.print("On");
    }
    else
    {
        printTarget.print("Off");
    }
}

SettingOnOff::SettingOnOff(const String *const Name) : Setting(Selection(1), Name)
{
}

// SettingFloat
SettingFloat::SettingFloat(const String *const Name, float Max) : Setting(Selection(FloatSettingToInt(Max), false), Name)
{
}

void SettingFloat::displayString(Print &printTarget)
{
    printTarget.print(*name);
    printTarget.print(' ');
    u8 after = 2;
    float val = getValue();
    if (val >= 10.0f)
        after--;
    if (val >= 100.0f)
        after--;
    printTarget.print(val, after);
}
float SettingFloat::getValue()
{
    return SettingIntToFloat(sel.getState());
}

// SettingFloat01
SettingFloat01::SettingFloat01(const String *const Name) : Setting(Selection(1000, false), Name)
{
}

void SettingFloat01::displayString(Print &printTarget)
{
    printTarget.print(*name);
    printTarget.print(' ');

    u16 st = sel.getState(); // 19580 B flash at this code version

    u8 d = st / 1000;
    printTarget.print(d);
    st -= d * 1000;

    printTarget.print('.');

    d = st / 100;
    printTarget.print(d);
    st -= d * 100;

    d = st / 10;
    printTarget.print(d);
    st -= d * 10;

    printTarget.print(st);
    // printTarget.print(sel.getState() * 0.001f, 3); //try without the "* 0.001f" for the funniest bug ever
    // 19750 B flash at this code version
}
float SettingFloat01::getValue()
{
    return sel.getState() * 0.001f;
}

// SettingMetricMultiplier
SettingMetricMultiplier::SettingMetricMultiplier(const String *const unit) : Setting(Selection(SUPPORTED_PREFIX_COUNT * 2), unit)
{
    sel.state = SUPPORTED_PREFIX_COUNT;
}

void SettingMetricMultiplier::displayString(Print &printTarget)
{
    u8 prefix = sel.getState();
    if (prefix == SUPPORTED_PREFIX_COUNT)
    {
        printTarget.print(' ');
    }
    else if (prefix > SUPPORTED_PREFIX_COUNT)
    {
        printTarget.print(metricPrefixes[prefix - 1]);
    }
    else
    {
        printTarget.print(metricPrefixes[SUPPORTED_PREFIX_COUNT - 1 - prefix]);
    }
    printTarget.print(*name); // unit
}

float SettingMetricMultiplier::getMultiplier()
{
    i8 prefix = sel.getState();
    prefix -= SUPPORTED_PREFIX_COUNT;
    float ret = 1;
    while (prefix > 0)
    {
        ret *= 1000;
        prefix--;
    }
    while (prefix < 0)
    {
        ret /= 1000;
        prefix++;
    }
    return ret;
}

float SettingMetricMultiplier::getCombined(SettingFloat &with)
{
    float ret = getMultiplier();
    return ret * with.getValue();
}
// SettingsEnum
void SettingEnum::displayString(Print &printTarget)
{
    u16 state = sel.getState();
    LinePrinter p;
    p.setLineOutput(printTarget, 0);
    p.print(*name);
    printTarget.print(' ');
    p.setLineOutput(printTarget, 1 + (i16)state); // cast ok because state is max 2047
    p.print(*name);
    while (p.hasPrinted == 0) // make sure no forbidden state is used, which is characterized by an empty string
    {
        sel.increment(1, true);
        p.setLineOutput(printTarget, 1 + (i16)sel.getState());
        p.print(*name);               // print it again to see if the string is empty again
        if (sel.getState() == state) // if after a full round nothing is found
        {
            break; // prevent infinite loop
        }
    }
}
SettingEnum::SettingEnum(const String *const NameAndEnumLines, bool allowOverflow) : Setting(Selection(2047, allowOverflow), NameAndEnumLines)
{
    LinePrinter p;
    p.startLineCounting();
    p.print(*name);
    sel.changeMax(p.newlines - 1);
}

// SettingMultiMode
SettingMultiMode::SettingMultiMode(const String *const NameAndEnumLines, SettingEnum &ModeSelect, bool allowOverflow) : Setting(Selection(2047, allowOverflow), NameAndEnumLines), modeSelect(ModeSelect)
{
    LinePrinter p;
    p.startLineCounting();
    p.print(*name);
    i16 lines = p.newlines + 1;
    u16 modes = modeSelect.getMax() + 1;
    sel.changeMax((lines / modes) - 2); //-1 for name, -1 for max index, not size
}
void SettingMultiMode::displayString(Print &printTarget)
{
    u16 state = sel.getState();
    u16 mode = modeSelect.getState();
    //u16 modes = modeSelect.getMax() + 1;
    u16 linesPerMode = getMax() + 2; //+1 for name, +1 for size, not max index
    i16 skip = mode * linesPerMode;
    LinePrinter p;
    p.setLineOutput(printTarget, skip);
    p.print(*name);
    printTarget.print(' ');

    // debug
    /*printTarget.print(mode);
    printTarget.print(',');
    printTarget.print(modes);
    printTarget.print(',');
    printTarget.print(linesPerMode);
    printTarget.print(',');
    printTarget.print(skip);
    printTarget.print(',');
    printTarget.print(skip + 1 + (i16)state);
    printTarget.print(',');*/

    p.setLineOutput(printTarget, skip + 1 + (i16)state);
    p.print(*name);
    while (p.hasPrinted == 0) // make sure no forbidden state is used, which is characterized by an empty string
    {
        sel.increment(1, true);
        p.setLineOutput(printTarget, skip + 1 + (i16)sel.getState());
        p.print(*name);               // print it again to see if the string is empty again
        if (sel.getState() == state) // if after a full round nothing is found
        {
            break; // prevent infinite loop
        }
    }
}