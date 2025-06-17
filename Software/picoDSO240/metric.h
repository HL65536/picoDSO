
#ifndef SRC_METRIC_H_
#define SRC_METRIC_H_

#include "shortNames.h"

#define SUPPORTED_PREFIX_COUNT 5


#ifdef IS_HARVARD_ARCH
    flashChar metricPrefixes[] = "munpfkMGTP";
    char readFlashChar(const char * ptr, u16 offset);
#else
    const char metricPrefixes[] = {'m','u','n','p','f','k','M','G','T','P'};
#endif


// precision = 1ms, range -32s to +32s
// suitable for (relative) times for GUI applications
class TimeLight
{
    i16 milliseconds;
public:
    TimeLight operator+(const TimeLight &other) const;
    TimeLight operator-(const TimeLight &other) const;
    void operator+=(const TimeLight &other);
    void operator-=(const TimeLight &other);

    TimeLight operator*(float scalar) const;
    void operator*=(float scalar);
    TimeLight operator/(float scalar) const;
    void operator/=(float scalar);

    TimeLight operator*(i16 scalar) const;
    void operator*=(i16 scalar);
    TimeLight operator/(i16 scalar) const;
    void operator/=(i16 scalar);

    bool operator<(const TimeLight &other) const;
    bool operator<=(const TimeLight &other) const;
    bool operator>(const TimeLight &other) const;
    bool operator>=(const TimeLight &other) const;
    bool operator!=(const TimeLight &other) const;
    bool operator==(const TimeLight &other) const;

    float toFloat(); //unit=seconds

    void overflowProtect();//maxes out if overflow occurred

    TimeLight(i16 ms=0);
};
TimeLight operator"" _s(long double s);
TimeLight operator"" _ms(long double ms);

class LoopTimer
{
    u32 lastTimestamp = 0;

public:
    u32 tickRaw();//microseconds
    float tick(bool replaceAnomalyValues=true);//return elapsed time in s (since last call)
};

//lower RAM usage, but limited range
class LoopTimerLight
{
    u16 lastTimestamp = 0;

public:
    TimeLight tick(); //return elapsed time in s (since last call)
};

template <typename I, typename F>
struct intfloat//derived from dwengine project
{
    I intpart;
    F floatpart; // [0 ; 1[

    intfloat();
    intfloat(const I &, const F &);
    intfloat(const F &);

    void correct(); //correct floatpart to [0 ; 1[
    void correctPos(); //correct floatpart to [0 ; 1[ while already >=0

    intfloat<I, F> operator+(const intfloat<I, F> &other) const;
    intfloat<I, F> operator-(const intfloat<I, F> &other) const;
    void operator+=(const intfloat<I, F> &other);
    void operator-=(const intfloat<I, F> &other);

    void operator+=(const F &other);
    void operator-=(const F &other);

    intfloat<I, F> operator*(float scalar) const;
    void operator*=(float scalar);
    intfloat<I, F> operator/(float scalar) const;
    void operator/=(float scalar);

    bool operator<(const intfloat<I, F> &other) const;
    bool operator<=(const intfloat<I, F> &other) const;
    bool operator>(const intfloat<I, F> &other) const;
    bool operator>=(const intfloat<I, F> &other) const;
    bool operator!=(const intfloat<I, F> &other) const;
    bool operator==(const intfloat<I, F> &other) const;

    bool isInvalid(bool strict = true); //non-strict returns only true if guaranteed to be invalid, strict=true means also return true if invalidity is likely

    F toFloat();
};

template <typename I, typename F>
inline intfloat<I, F>::intfloat() : intpart(0), floatpart(0.0) {}

template <typename I, typename F>
inline intfloat<I, F>::intfloat(const I &i, const F &f) : intpart(i), floatpart(f) {}
template <typename I, typename F>
inline intfloat<I, F>::intfloat(const F &f) : intpart(0), floatpart(f)
{
    correct();
}

template <typename I, typename F>
inline intfloat<I, F> intfloat<I, F>::operator+(const intfloat<I, F> &other) const
{
    intfloat<I, F> ret;
    ret.floatpart = floatpart + other.floatpart;
    ret.intpart = intpart + other.intpart;
    ret.correctPos();
    return ret;
}

template <typename I, typename F>
inline intfloat<I, F> intfloat<I, F>::operator-(const intfloat<I, F> &other) const
{
    intfloat<I, F> ret;
    ret.floatpart = floatpart - other.floatpart;
    ret.intpart = intpart - other.intpart;
    ret.correct();
    return ret;
}

template <typename I, typename F>
inline void intfloat<I, F>::operator+=(const intfloat<I, F> &other)
{
    intpart += other.intpart;
    floatpart += other.floatpart;
    correctPos();
}

template <typename I, typename F>
inline void intfloat<I, F>::operator-=(const intfloat<I, F> &other)
{
    intpart -= other.intpart;
    floatpart -= other.floatpart;
    correct();
}
template <typename I, typename F>
inline void intfloat<I, F>::operator+=(const F &other)
{
    floatpart += other;
    correct();
}

template <typename I, typename F>
inline void intfloat<I, F>::operator-=(const F &other)
{
    floatpart -= other;
    correct();
}

template <typename I, typename F>
inline bool intfloat<I, F>::operator<(const intfloat<I, F> &other) const
{
    return (intpart < other.intpart) | ((intpart == other.intpart) & (floatpart < other.floatpart));
}

template <typename I, typename F>
inline bool intfloat<I, F>::operator<=(const intfloat<I, F> &other) const
{
    return (intpart < other.intpart) | ((intpart == other.intpart) & (floatpart <= other.floatpart));
}

template <typename I, typename F>
inline bool intfloat<I, F>::operator>(const intfloat<I, F> &other) const
{
    return (intpart > other.intpart) | ((intpart == other.intpart) & (floatpart > other.floatpart));
}

template <typename I, typename F>
inline bool intfloat<I, F>::operator>=(const intfloat<I, F> &other) const
{
    return (intpart > other.intpart) | ((intpart == other.intpart) & (floatpart >= other.floatpart));
}

template <typename I, typename F>
inline bool intfloat<I, F>::operator!=(const intfloat<I, F> &other) const
{
    return (intpart != other.intpart) | (floatpart != other.floatpart);
}

template <typename I, typename F>
inline intfloat<I, F> intfloat<I, F>::operator*(float scalar) const
{ //TODO make better
    intfloat<I, F> ret;
    ret.intpart = 0;
    ret.floatpart = scalar * floatpart;
    ret.correct();
    intfloat<I, F> ret2;
    ret2.intpart = 0;
    ret2.floatpart = scalar * intpart;
    ret2.correct();
    return ret + ret2;
}

template <typename I, typename F>
inline void intfloat<I, F>::operator*=(float scalar)
{
    *this = *this * scalar;
}

template <typename I, typename F>
inline intfloat<I, F> intfloat<I, F>::operator/(float scalar) const
{
    return *this * (1.0f / scalar);
}

template <typename I, typename F>
inline void intfloat<I, F>::operator/=(float scalar)
{
    *this *= (1.0f / scalar);
}

template <typename I, typename F>
inline bool intfloat<I, F>::operator==(const intfloat<I, F> &other) const
{
    return (intpart == other.intpart) & (floatpart == other.floatpart);
}

template <typename I, typename F>
inline void intfloat<I, F>::correct()
{
    I change = (I)floatpart;
    change -= (floatpart < 0);//correction because (int)(-0.5) is 0
    intpart += change;
    floatpart -= change;
}

template <typename I, typename F>
inline void intfloat<I, F>::correctPos()
{
    I change = (I)floatpart;
    intpart += change;
    floatpart -= change;
}
template <typename I, typename F>
inline F intfloat<I, F>::toFloat()
{
    return floatpart+(F)intpart;
}

typedef intfloat<i32, float> Charge;
typedef Charge Energy;
typedef Charge Time;

void metricPrintOld(float value, u8 sigDigits = 3, bool space = true);

void metricPrint(float value, Print &printTarget = Serial, i8 sigDigits = 3, bool space = true); //sigDigits must be 1-9 //TODO test values <3

void metricPrintln(float value, char unit, Print &printTarget = Serial, u8 sigDigits = 3, bool space = true);

void metricPrintln(Charge value, char unit, Print &printTarget = Serial, u8 sigDigits = 3, bool space = true);
//these 2 should also work for energy units
void hourPrintln(Charge value, char unit, Print &printTarget = Serial, u8 sigDigits = 3, bool space = true);//for e.g. printing mAh while code uses coulombs
void hourPrint(Charge value, char unit, Print &printTarget = Serial, u8 sigDigits = 3, bool space = true); // for e.g. printing mAh while code uses coulombs

void timePrintln(Time value, Print & printTarget=Serial);

#endif /* SRC_METRIC_H_ */