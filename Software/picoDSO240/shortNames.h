
#ifndef SRC_SHORTNAMES_H_
#define SRC_SHORTNAMES_H_

#include <Arduino.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;

//#ifdef ARDUINO_ARCH_RP2040
//TODO use these if already defined error shows up again
//typedef uint16_t u16;//already defined????
//#endif               /* u16 */
//#ifdef _AVR_ATTINY402_H_INCLUDED
//typedef uint16_t u16;
//#endif                /* u16 */
//#ifdef __AVR_ATMEGA4809__
//typedef uint16_t u16;
//#endif                /* u16 */
#ifndef ARDUINO_AVR_UNO
typedef uint16_t u16;
#endif                /* u16 */

typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

#define u64(x) ((u64)(x))
#define i64(x) ((i64)(x))
#define u32(x) ((u32)(x))
#define i32(x) ((i32)(x))
#define u16(x) ((u16)(x))
#define i16(x) ((i16)(x))
#define u8(x) ((u8)(x))
#define i8(x) ((i8)(x))

//change stuff here to specify architecture type for other µCs, as multiple other libraries (can) use it
#ifdef ARDUINO_ARCH_RP2040
#define IS_VON_NEUMANN_ARCH // defined as being able to execute code from RAM
#else
    #define IS_HARVARD_ARCH // defined as not being able to execute code from RAM
#endif



#ifdef IS_HARVARD_ARCH
    //typedef const char flashString[] PROGMEM;//does not work
    #define flashChar \
        const PROGMEM char
#endif



#endif /* SRC_SHORTNAMES_H_ */