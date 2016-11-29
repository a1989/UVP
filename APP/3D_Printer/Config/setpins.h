#ifndef __SETPINS_H
#define __SETPINS_H

#include "stm32f4xx.h"

void IO_Init(void);
void enable_x(void);
void enable_y(void);
void enable_z(void);
void enable_e0(void);
void enable_e1(void);
void enable_e2(void);

void WRITE_Pin(GPIO_TypeDef* Port, uint16_t Pin, BitAction BitVal);
uint8_t READ_Pin(GPIO_TypeDef* Port, uint16_t Pin);
void st_init(void);
void tp_init(void);

#endif
