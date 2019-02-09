#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG 1

#include "application.h"

#if DEBUG == 1 && !defined(IS_WINDOWS)
#define PN532DEBUG 1
#define DMSG(args...)       Serial.print(args)
#define DMSG_STR(str)       Serial.println(str)
#define DMSG_HEX(num)       Serial.print(" "); Serial.print(num, HEX)
#define DMSG_INT(num)       Serial.print(" "); Serial.print(num)
#else
#if defined(IS_WINDOWS)
#define DMSG(...)
#else
#define DMSG(args...)
#endif
#define DMSG_STR(str)
#define DMSG_HEX(num)
#define DMSG_INT(num)
#endif

#endif
