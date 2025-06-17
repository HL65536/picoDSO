#ifndef SRC_EMBEDUI_VONNEUMANNARCH_H_
#define SRC_EMBEDUI_VONNEUMANNARCH_H_

//TODO port to von Neumann architecture
class Setting
{
protected:
    Selection sel;
    const String * const name;//yes, a const pointer to a const string. Purpose: make compiler put the string into flash

public:
    u16 getMax();
    u16 getState();

    //u16 storeEEprom(u16 startIndex); // return endIndex
    //u16 loadEEprom(u16 startIndex);  // return endIndex

    void setState(u16 newState); // try to avoid, does not check bounds
    void setMax(u16 newMax);     // try to avoid, does not check bounds & conflicts
    void tick(SelectButtons &buts, TimeLight timeElapsed, bool wasReset = 0);
    virtual void displayString(Print &printTarget); // name + value
    Setting(Selection Sel, const String *const Name);
};

class SettingInt : public Setting
{
public:
    SettingInt(u16 max, const String *const Name);
};

class SettingOnOff : public Setting
{
public:
    void displayString(Print &printTarget) override;

    SettingOnOff(const String *const Name);
};

class SettingFloat : public Setting
{
public:
    void displayString(Print &printTarget) override;

    float getValue();

    SettingFloat(const String *const Name, float Max = 1000.0f);
};

class SettingMetricMultiplier : public Setting
{
public:
    void displayString(Print &printTarget) override;
    float getMultiplier();
    float getCombined(SettingFloat &with);

    SettingMetricMultiplier(const String *const unit);
};

class SettingFloat01 : public Setting
{
public:
    void displayString(Print &printTarget) override;

    float getValue();

    SettingFloat01(const String *const Name);
};

class SettingEnum : public Setting
{
public:
    void displayString(Print &printTarget) override;

    SettingEnum(const String *const NameAndEnumLines, bool allowOverflow = true);
};

class SettingMultiMode : public Setting
{
    SettingEnum &modeSelect;

public:
    void displayString(Print &printTarget) override;

    SettingMultiMode(const String *const NameAndEnumLines, SettingEnum &ModeSelect, bool allowOverflow = true);
};





#endif /* SRC_EMBEDUI_VONNEUMANNARCH_H_ */