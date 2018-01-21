#ifndef FAKE_Arduino_h
#define FAKE_Arduino_h

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <iostream>

#include "itoa.h"
#include "WString.h"

#define HIGH 0x1
#define LOW  0x0

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

#define INTERNAL 3
#define DEFAULT 1
#define EXTERNAL 0

#define OUTPUT 0
#define INPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#define PIN_MODE_NONE 0xff

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#if !defined(_WIN32) && !defined(WIN32)
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#endif
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b) (1UL << (b))

typedef unsigned int word;
typedef uint8_t byte;

#ifdef __cplusplus
extern "C" {
#endif

void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);
int analogRead(uint8_t);
unsigned long millis();
void analogWrite(uint8_t, int);
void delay(unsigned long);

#ifdef __cplusplus
}
#endif

//DECLARE_FAKE_VOID_FUNC( pinMode, uint8_t, uint8_t );
//DECLARE_FAKE_VOID_FUNC( digitalWrite, uint8_t, uint8_t );
//DECLARE_FAKE_VALUE_FUNC( int, digitalRead, uint8_t );
//DECLARE_FAKE_VALUE_FUNC( unsigned long, millis );
//DECLARE_FAKE_VALUE_FUNC( int, analogRead, uint8_t );
//DECLARE_FAKE_VOID_FUNC( analogWrite, uint8_t, int );
//DECLARE_FAKE_VOID_FUNC( delay, unsigned long );



#include <queue>
#include <string>
#include <map>


class Serial_CLS
{
   typedef std::queue<std::string> Buffer;

   public:
	   void write(char value) { }
	   void write( const char* buffer, int buffer_n ) { }
      void write( const char* buffer ) { }
	  void print(unsigned int value) { }
	  void print( int value ) { }
      void print( double value ) { }
	  void print(const char* buffer) { }
	  void print(String buffer) { }
	  void print(byte value, byte format) { }
	  void println(const char* buffer) { }
	  void println(String buffer) { }
	  void println() { }
	  void println(unsigned int value) { }
	  void println(int value) { }
	  void println(float value) { }
	  void println(byte value) { }
	  void println(byte value, byte format) { }
	  template<typename... Args>
	  void printf(const char *fmt, Args ...args) { }

      void begin( int baudrate ) {}
	  int available() { return 0; }
	  char read() { return 'a'; }
	  void flush() {}

      // any printing will be appended to this vector
      Buffer _test_output_buffer;
      std::string _test_output_current; // current output line
      std::queue<char> _test_input_buffer;

      void _test_clear(); // remove and reset everything from the buffers
      void _test_set_input( const char* what );
   protected:
      void _test_output_string( std::string what );
};

extern Serial_CLS Serial;

static const int A0 = 100;

#define LED_BUILTIN 13


#endif
