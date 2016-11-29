#ifndef __STEPPER_H
#define __STEPPER_H

#include "stm32f4xx.h"
//#include "defines.h"



#define MultiU24X24toH16(intRes, longIn1, longIn2)		(intRes) = ( (longIn1) * (longIn2) ) >> 24
#define MultiU16X8toH16(intRes, intIn1, intIn2)			(intRes) = ( ((intIn1) * (intIn2)) >> 8 )




void trapezoid_generator_reset(void);
void st_set_position(const long x, const long y, const long z, const long e);
unsigned short calc_timer(unsigned short step_rate);
void st_set_e_position(const long e);

#endif
