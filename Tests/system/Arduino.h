#ifndef FAKE_Arduino_h
#define FAKE_Arduino_h

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "fff.h"

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
typedef bool boolean;
typedef uint8_t byte;


void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);
int analogRead(uint8_t);
unsigned long millis();
void analogWrite(uint8_t, int);
void delay(unsigned long);

DECLARE_FAKE_VOID_FUNC( pinMode, uint8_t, uint8_t );
DECLARE_FAKE_VOID_FUNC( digitalWrite, uint8_t, uint8_t );
DECLARE_FAKE_VALUE_FUNC( int, digitalRead, uint8_t );
DECLARE_FAKE_VALUE_FUNC( unsigned long, millis );
DECLARE_FAKE_VALUE_FUNC( int, analogRead, uint8_t );
DECLARE_FAKE_VOID_FUNC( analogWrite, uint8_t, int );
DECLARE_FAKE_VOID_FUNC( delay, unsigned long );



#include <queue>
#include <string>
#include <map>


class Serial_CLS
{

   typedef std::queue<std::string> Buffer;

   public:
      void write( const char* buffer, int buffer_n );
      void write( const char* buffer );
      void print( int value );
      void print( double value );
	  void print(const char* buffer);
	  void print(byte value, byte format);
	  void println(const char* buffer);
	  void println(int value);
	  void println(byte value, byte format);
      void begin( int baudrate );
      int available();
      char read();

      // any printing will be appended to this vector
      Buffer _test_output_buffer;
      std::string _test_output_current; // current output line
      std::queue<char> _test_input_buffer;

      void _test_clear(); // remove and reset everything from the buffers
      void _test_set_input( const char* what );
   protected:
      void _test_output_string( std::string what );
};


class Arduino_TEST
{
   public:
     enum class  Check_mode { Full, None }; // Mode FULL for all checks (check that digital write is output and digital read is input), Defaults to FULL
     constexpr static const int MAX_PINS = 128;

     void hookup(); // Reset and hook the arduino functions (digitalRead, digitalWrite, pinMode, analogRead, analogWrite)
     void hookdown(); // clear all values and custom hookups.
     void set_mode( Check_mode target );
     void check_write(uint8_t pin);
     void check_read(uint8_t pin);
     void check_pin(uint8_t pin);
     int     pin_value[ MAX_PINS ];
     uint8_t pin_mode [ MAX_PINS ];

     Check_mode check_mode;
};

extern Arduino_TEST ARDUINO_TEST;

extern Serial_CLS Serial;

static const int A0 = 100;

#define LED_BUILTIN 13


#endif
