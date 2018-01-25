#include "application.h"

#ifdef __cplusplus
extern "C" {
#endif

PinFunction HAL_Validate_Pin_Function(pin_t pin, PinFunction pinFunction) { return PF_NONE; }

STM32_Pin_Info pinInfo[1];
STM32_Pin_Info* HAL_Pin_Map(void) { 
  GPIO_TypeDef peripheral;
  pinInfo[0].gpio_peripheral = &peripheral; 
  return pinInfo; 
}

void HAL_Pin_Mode(pin_t pin, PinMode mode) {}
PinMode HAL_Get_Pin_Mode(pin_t pin) { return PIN_MODE_NONE;  }
void HAL_GPIO_Write(pin_t pin, uint8_t value) {}
int32_t HAL_GPIO_Read(pin_t pin) { return 0; }
uint32_t HAL_Pulse_In(pin_t pin, uint16_t value) { return 0; }
void noInterrupts() {}
void interrupts() {}
void delayMicroseconds(uint ms) {}

#ifdef __cplusplus
}
#endif

_RGB RGB;
TwoWire Wire;
SPIClass SPI;
_Particle Particle;
_Time Time;
_System System;
Pin_Info PIN_MAP[];
