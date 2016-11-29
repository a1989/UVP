#ifndef __FUNCTION_H
#define __FUNCTION_H

//#include "scarausart.h"
//#include "scaralcd.h"
#include "setpins.h"
#include "stdbool.h"
#include "stdlib.h"
#include "setpins.h"
//#include "GUI.h"
//#include "stm32f429i_discovery_ioe.h"
#include "misc.h"
#include "stm32f4xx_tim.h"
#include "defines.h"



enum MENUS{
	MAIN_MENU = 0,
	READ_SDCARD_MENU,
	CONTROL_MENU,
	PREPARE_MENU,
	UTILITIES_MENU,
};

typedef struct TOUCH_STAT{
		uint8_t IS_Pressed;
		uint32_t t_x;
		uint32_t t_y;
}Touch_Stat;






void Move_Axis_Home(int Axis);
void Move_Axis_mm_positive(int Axis, int Speed);
void Move_Axis_mm_negative(int Axis, int Speed);
void TIM_Init(void);

void scara_setup(void);
void scara_settings(void);
void scara_operation_task(void);
void Move_Axis_mm(int Axis, int direction);

#endif
