#ifndef __TEMPERATURE_H
#define __TEMPERATRUE_H

#include "stm32f4xx.h"

void setTargetBed(const float celsius); 
float degHotend(uint8_t extruder);
float degTargetHotend(uint8_t extruder);
float degBed(void);
float degTargetBed(void);
int getHeaterPower(int heater);
void manage_heater(void);
void max_temp_error(uint8_t e);
void min_temp_error(uint8_t e);
void bed_max_temp_error(void);
void setTargetHotend(const float celsius, uint8_t extruder);

#endif
