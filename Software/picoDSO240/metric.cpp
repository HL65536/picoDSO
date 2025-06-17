#include "metric.h"


#ifdef ARDUINO_ARCH_RP2040
//#include <cmath.h>
using namespace std;
#else
/*bool isnan(float val)
{
    if(val>0)//NaNs never return true on comparisons
    {
        return false;
    }
    if(val<=0)//NaNs never return true on comparisons
    {
        return false;
    }
    return true;
}
bool isfinite(float val)
{

}*/
#endif

//LoopTimer
float LoopTimer::tick(bool replaceAnomalyValues)
{
    u32 newTimestamp = micros();
    float ret = newTimestamp - lastTimestamp;
    lastTimestamp = newTimestamp;
    ret *= 0.000001f; // timestamp was in microseconds
    if(replaceAnomalyValues)
    {
        bool isNotNormal;

#ifdef ARDUINO_ARCH_RP2040
        isNotNormal=!isnormal(ret);
#else
        isNotNormal=(ret==0);
        if(isnan(ret)) isNotNormal=true;
        if(isinf(ret)) isNotNormal=true;//no protection from near-zero values, as isnormal does not work
#endif

        if(isNotNormal)
        {
            ret = 0.000001f;
        }
    }
    return ret;
}
u32 LoopTimer::tickRaw()
{
    u32 newTimestamp = micros();
    u32 ret = newTimestamp - lastTimestamp;
    lastTimestamp = newTimestamp;
    return ret;
}

//LoopTimerLight
TimeLight LoopTimerLight::tick()
{
    u16 newTimestamp = millis();
    u16 ret = newTimestamp - lastTimestamp;
    lastTimestamp = newTimestamp;
    return TimeLight((i16)ret);
}

//TimeLight
TimeLight::TimeLight(i16 ms):
milliseconds(ms)
{}

TimeLight operator"" _s(long double s)
{
    return TimeLight(i16(s*1000));
}

TimeLight operator"" _ms(long double ms)
{
    return TimeLight(i16(ms));
}

TimeLight TimeLight::operator+(const TimeLight &other) const
{
    return TimeLight(milliseconds + other.milliseconds);
}

TimeLight TimeLight::operator-(const TimeLight &other) const
{
    return TimeLight(milliseconds - other.milliseconds);
}

void TimeLight::operator+=(const TimeLight &other)
{
    milliseconds+=other.milliseconds;
}

void TimeLight::operator-=(const TimeLight &other)
{
    milliseconds -= other.milliseconds;
}

TimeLight TimeLight::operator*(float scalar) const
{
    return TimeLight(milliseconds * scalar);
}

void TimeLight::operator*=(float scalar)
{
    milliseconds *= scalar;
}

TimeLight TimeLight::operator/(float scalar) const
{
    return TimeLight(milliseconds / scalar);
}

void TimeLight::operator/=(float scalar)
{
    milliseconds /= scalar;
}

TimeLight TimeLight::operator*(i16 scalar) const
{
    return TimeLight(milliseconds * scalar);
}

void TimeLight::operator*=(i16 scalar)
{
    milliseconds *= scalar;
}

TimeLight TimeLight::operator/(i16 scalar) const
{
    return TimeLight(milliseconds / scalar);
}

void TimeLight::operator/=(i16 scalar)
{
    milliseconds /= scalar;
}

bool TimeLight::operator<(const TimeLight &other) const
{
    return milliseconds < other.milliseconds;
}

bool TimeLight::operator<=(const TimeLight &other) const
{
    return milliseconds <= other.milliseconds;
}

bool TimeLight::operator>(const TimeLight &other) const
{
    return milliseconds > other.milliseconds;
}

bool TimeLight::operator>=(const TimeLight &other) const
{
    return milliseconds >= other.milliseconds;
}

bool TimeLight::operator!=(const TimeLight &other) const
{
    return milliseconds != other.milliseconds;
}

bool TimeLight::operator==(const TimeLight &other) const
{
    return milliseconds == other.milliseconds;
}

float TimeLight::toFloat()
{
    return milliseconds * 0.001f;
}

void TimeLight::overflowProtect()
{
    if(milliseconds<0)
    {
        milliseconds=32767;
    }
}

//functions
char readFlashChar(const char *ptr, u16 offset)
{
    PGM_P p = reinterpret_cast<PGM_P>(ptr);
    p += offset;
    unsigned char c = pgm_read_byte(p);
    return c;
}

void metricPrint(float value, Print &printTarget, i8 sigDigits, bool space) //sigDigits must be 1-9 //TODO test values <3
{
    if(isnan(value))
    {
        printTarget.print("NaN");
        if (space)
        {
            printTarget.print(' ');
        }
        return;
    }
    if (!isfinite(value))
    {
        if (value < 0)
        {
            printTarget.print('-');
        }
        printTarget.print("inf");
        if (space)
        {
            printTarget.print(' ');
        }
        return;
    }
    u8 xlShift=0;
    while(value>1073741824.0f)
    {
        value/=1000;
        xlShift++;
    }
    if (value < 0)
    {
        value = -value; //print '-' now and forget it was ever negative
        printTarget.print('-');
    }
    else if (value == 0)
    {
        printTarget.print('0');
        if (space)
        {
            printTarget.print(' ');
        }
        return; //ensure value is >0 for following operations
    }

    i32 minVal = 1;
    //value has to be between these 2
    i32 tooMuchVal = 10;

    for (i8 i = 1; i < sigDigits; i++) //yes, i=1, because the values are correct if sigDigits==1
    {
        minVal = tooMuchVal; //multiply everything by 10 for every significant digit above 1
        tooMuchVal *= 10;
    }               //now defines a range for raw digits: must form a number between [minVal ; tooMuchVal[
    i8 shifted = 0; //how much the dot is shifted to fit value in above mentioned range
    while (i32(value) >= tooMuchVal)
    {
        value /= 10;
        shifted--;
    }
    while (i32(value) < minVal)
    {
        value *= 10;
        shifted++;
    }
    i32 valueRounded = i32(value + 0.5f); //now contains all significant digits to be printed
    i8 preDot = sigDigits - shifted;      //current digits before the dot

    i8 prefix = 0; //1=K 2=M 3=G ...
    while (preDot > 3)
    {
        preDot -= 3;
        prefix++;
    }
    while (preDot < 1)
    {
        preDot += 3;
        prefix--;
    }
    //now there are 1/2/3 digits before the dot
    for (i8 i = 0; i < preDot; i++)
    {
        printTarget.print(valueRounded / minVal); //print digits before dot
        valueRounded %= minVal;
        minVal /= 10;
    }
    if (minVal > 0)
        printTarget.print('.');
    while (minVal > 0)
    {
        printTarget.print(valueRounded / minVal); //print digits after dot
        valueRounded %= minVal;
        minVal /= 10;
    }

    if (space)
    {
        printTarget.print(' ');
    }
    prefix+=xlShift;
    if (prefix > 0)
    {
        prefix--; //for zero-based index
        if (prefix >= SUPPORTED_PREFIX_COUNT)
            printTarget.print('?');
        else
            printTarget.print(readFlashChar(metricPrefixes, prefix + SUPPORTED_PREFIX_COUNT));
    }
    else if (prefix < 0)
    {
        prefix = -prefix;
        prefix--; //for zero-based index
        if (prefix >= SUPPORTED_PREFIX_COUNT)
            printTarget.print('?');
        else
            printTarget.print(readFlashChar(metricPrefixes, prefix));
    }
}

void metricPrintln(float value, char unit, Print &printTarget, u8 sigDigits, bool space)
{
    metricPrint(value, printTarget, sigDigits, space);
    printTarget.println(unit);
}

void metricPrintln(Charge value, char unit, Print &printTarget, u8 sigDigits, bool space)
{
    metricPrintln(value.toFloat(), unit, printTarget, sigDigits, space);
}
void hourPrintln(Charge value, char unit, Print &printTarget, u8 sigDigits, bool space)
{
    metricPrint(value.toFloat() / 3600.0f, printTarget, sigDigits, space);
    printTarget.print(unit);
    printTarget.println('h');
}
void hourPrint(Charge value, char unit, Print &printTarget, u8 sigDigits, bool space)
{
    metricPrint(value.toFloat() / 3600.0f, printTarget, sigDigits, space);
    printTarget.print(unit);
    printTarget.print('h');
}

void timePrintln(Time value, Print &printTarget)
{
    i32 val = value.intpart;
    u8 seconds = val % 60;
    val /= 60;
    u8 minutes = val % 60;
    val /= 60;
    u8 hours = val % 24;
    val /= 24;
    printTarget.print(val);
    printTarget.print('d');
    printTarget.print(' ');
    printTarget.print(hours / 10);
    printTarget.print(hours % 10);
    printTarget.print(':');
    printTarget.print(minutes / 10);
    printTarget.print(minutes % 10);
    printTarget.print(':');
    printTarget.print(seconds / 10);
    printTarget.println(seconds % 10);
}
