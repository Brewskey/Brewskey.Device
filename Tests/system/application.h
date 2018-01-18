#ifndef FAKE_application_h
#define FAKE_application_h

#include <functional>

#include "Arduino.h"
#include "Servo.h"

#define PRODUCT_ID(v)
#define PRODUCT_VERSION(v)

#define Serial1 Serial
#define uint unsigned int

class _RGB {
public:
  void control(bool val) {};
  void color(char r, char g, char b) {};
};
extern _RGB RGB;

class _Particle {
public:
	void syncTime() {};
};

extern _Particle Particle;

class Print {
};

class SPIClass {

};

class Timer {
	typedef std::function<void(void)> timer_callback_fn;

	Timer(unsigned period, timer_callback_fn callback_, bool one_shot = false) {
	}

	template <typename T>
	Timer(unsigned period, void (T::*handler)(), T& instance, bool one_shot = false) : Timer(period, std::bind(handler, &instance), one_shot)
	{
	}
};

#if defined(_WIN32) || defined(WIN32) 
#define __attribute__(A)
#endif

#endif
