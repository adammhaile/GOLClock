#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Arduino Uno
#define __AVR_ATmega328P__
#define 
#define ARDUINO 105
#define ARDUINO_MAIN
#define __AVR__
#define F_CPU 16000000L
#define __cplusplus
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define __attribute__(x)
#define NOINLINE __attribute__((noinline))
#define prog_void
#define PGM_VOID_P int
            
typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

//
//
void backwardClock(DateTime now);
void forwardClock(DateTime now);
void staticClock(DateTime now);
void meltClock(DateTime now);
void brightClock(DateTime now);
void chartClock(DateTime now);
void binaryClock(DateTime now);
void epochClock(DateTime now);
uint8_t meltPixel (uint8_t x, uint8_t y, uint8_t z, uint8_t seed);
bool randPixel (uint8_t x, uint8_t y, uint8_t z, uint8_t seed);
void cycle(byte bigsteps);
void displayFrame();
void clearFrame();
int neighbours(int x, int y);
void displayTime(DateTime now);

#include "C:\Program Files (x86)\arduino\hardware\arduino\variants\standard\pins_arduino.h" 
#include "C:\Program Files (x86)\arduino\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\Kredal\Documents\Arduino\kredal\trunk\GOLClock\GOLClock.ino"
#include "C:\Users\Kredal\Documents\Arduino\kredal\trunk\GOLClock\font.h"
#include "C:\Users\Kredal\Documents\Arduino\kredal\trunk\GOLClock\globals.h"
#include "C:\Users\Kredal\Documents\Arduino\kredal\trunk\GOLClock\numbers.h"
#include "C:\Users\Kredal\Documents\Arduino\kredal\trunk\GOLClock\settime.h"
#endif
