#ifndef __DEFINES_H
#define __DEFINES_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
//bool true = 1;
//bool false = 0;

#define BUFSIZE 4
#define MAX_CMD_SIZE 96

#define NUM_AXIS		4

#define SCARA_offset_x  107
#define SCARA_offset_y  308
//#define Linkage_1		188000
//#define Linkage_2		128000

//#define Linkage_1		188
//#define Linkage_2		128

#define Linkage_1		170
#define Linkage_2		170

#define SCARA_RAD2DEG		57.29

#define X_MAX_POS 214
#define X_MIN_POS 0
#define Y_MAX_POS 214
#define Y_MIN_POS 0
#define Z_MAX_POS 300
#define Z_MIN_POS 0

#define min_software_endstops true
#define max_software_endstops true

#define DELTA_SEGMENT_PER_SECOND 120


#define MINIMUM_PLANNER_SPEED 0.05
#define square(x) ((x) * (x))

#define HOMING_FEEDRATE {50*6, 50*6, 20*60, 0}

enum AxisEnum {X_AXIS = 0, Y_AXIS, Z_AXIS, E_AXIS};

static float previous_speed[4];
static float previous_nominal_speed;

#define MAX_STEP_FREQUENCY 40000
#define F_CPU 1000000UL


#define X_DIR_PIN_PORT			GPIOB
#define X_DIR_PIN						GPIO_Pin_9
#define X_STEP_PIN_PORT			GPIOB
#define X_STEP_PIN        	GPIO_Pin_8
#define X_ENABLE_PIN_PORT		GPIOB
#define X_ENABLE_PIN				GPIO_Pin_7

#define Y_DIR_PIN_PORT			GPIOF
#define Y_DIR_PIN						GPIO_Pin_11
#define Y_STEP_PIN_PORT			GPIOC
#define Y_STEP_PIN        	GPIO_Pin_6
#define Y_ENABLE_PIN_PORT		GPIOF
#define Y_ENABLE_PIN				GPIO_Pin_11

//#define Y_DIR_PIN_PORT			GPIOC
//#define Y_DIR_PIN						GPIO_Pin_13
//#define Y_STEP_PIN_PORT			GPIOC
//#define Y_STEP_PIN        	GPIO_Pin_3
//#define Y_ENABLE_PIN_PORT		GPIOA
//#define Y_ENABLE_PIN				GPIO_Pin_0

//#define Z_DIR_PIN_PORT			GPIOA
//#define Z_DIR_PIN						GPIO_Pin_15
//#define Z_STEP_PIN_PORT			GPIOB
//#define Z_STEP_PIN					GPIO_Pin_3
//#define Z_ENABLE_PIN_PORT		GPIOA
//#define Z_ENABLE_PIN				GPIO_Pin_6

#define Z_DIR_PIN_PORT			GPIOF
#define Z_DIR_PIN						GPIO_Pin_8
#define Z_STEP_PIN_PORT			GPIOF
#define Z_STEP_PIN					GPIO_Pin_7
#define Z_ENABLE_PIN_PORT		GPIOF
#define Z_ENABLE_PIN				GPIO_Pin_10

/*
#define X_MIN_PIN_PORT			GPIOA
//#define X_MIN_PIN						GPIO_Pin_5
#define X_MIN_PIN						GPIO_Pin_1
#define Y_MIN_PIN_PORT			GPIOA
#define Y_MIN_PIN						GPIO_Pin_7
#define Z_MIN_PIN_PORT			GPIOB
#define Z_MIN_PIN						GPIO_Pin_12

#define X_MAX_PIN_PORT      (void *)0
#define X_MAX_PIN         	-1 
#define Y_MAX_PIN_PORT      (void *)0
#define Y_MAX_PIN						-1
#define Z_MAX_PIN_PORT      (void *)0
#define Z_MAX_PIN						-1
*/

#define X_MIN_PIN_PORT			(void *)0
//#define X_MIN_PIN						GPIO_Pin_5
#define X_MIN_PIN						-1
#define Y_MIN_PIN_PORT			(void *)0
#define Y_MIN_PIN						-1
#define Z_MIN_PIN_PORT			GPIOF
#define Z_MIN_PIN						GPIO_Pin_9
//#define Z_MIN_PIN_PORT			GPIOF
//#define Z_MIN_PIN						GPIO_Pin_5

#define X_MAX_PIN_PORT      GPIOB
#define X_MAX_PIN         	GPIO_Pin_6
#define Y_MAX_PIN_PORT      GPIOB
#define Y_MAX_PIN						GPIO_Pin_1

//#define Y_MAX_PIN_PORT      GPIOE
//#define Y_MAX_PIN						GPIO_Pin_4

#define Z_MAX_PIN_PORT      (void *)0
#define Z_MAX_PIN						-1

#define E0_ENABLE_PIN_PORT		GPIOA
#define E0_ENABLE_PIN					GPIO_Pin_11
#define E0_STEP_PIN_PORT			GPIOA
#define E0_STEP_PIN						GPIO_Pin_9
#define E0_DIR_PIN_PORT				GPIOB
#define E0_DIR_PIN						GPIO_Pin_10


#define HEATER_END_PIN_PORT		GPIOB
#define HEATER_END_PIN				GPIO_Pin_4

#define HEATER_BED_PIN_PORT		GPIOA
#define HEATER_BED_PIN				GPIO_Pin_5

#define TEMP_0_PIN_PORT				GPIOC
#define TEMP_0_PIN						GPIO_Pin_0

#define TEMP_BED_PIN_PORT			GPIOC
#define TEMP_BED_PIN					GPIO_Pin_2

#define INVERT_X_DIR 		false    // for Mendel set to false, for Orca set to true
#define INVERT_Y_DIR 		false    // for Mendel set to true, for Orca set to false
#define INVERT_Z_DIR 		true    // for Mendel set to false, for Orca, Morgan set to true
#define INVERT_E0_DIR 	false   // for direct drive extruder v9 set to true, for geared extruder set to false
#define INVERT_E1_DIR 	false   // for direct drive extruder v9 set to true, for geared extruder set to false
#define INVERT_E2_DIR 	false   // for direct drive extruder v9 set to true, for geared extruder set to false

   

#define INVERT_X_STEP_PIN 	false
#define INVERT_Y_STEP_PIN 	false
#define INVERT_Z_STEP_PIN 	false
#define INVERT_E_STEP_PIN 	false

#define HIGH		true
#define LOW			false

#define X_ENABLE_ON false
#define Y_ENABLE_ON false
#define Z_ENABLE_ON false
#define E_ENABLE_ON false

#define EXTRUDERS 1
static bool old_x_min_endstop = false;
static bool old_x_max_endstop = false;
static bool old_y_min_endstop = false;
static bool old_y_max_endstop = false;
static bool old_z_min_endstop = false;
static bool old_z_max_endstop = false;



static volatile bool endstop_x_hit=false;
static volatile bool endstop_y_hit=false;
static volatile bool endstop_z_hit=false;

static bool check_endstops = true;


#define X_MAX_LENGTH (X_MAX_POS - X_MIN_POS)
#define Y_MAX_LENGTH (Y_MAX_POS - Y_MIN_POS)
#define Z_MAX_LENGTH (Z_MAX_POS - Z_MIN_POS)

#define home_retract_mm 	5
//#define home_dir					-1

//#define   CRITICAL_SECTION_START 	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE)//asm("CPSID  I")
//#define		CRITICAL_SECTION_END		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE)//asm("CPSIE  I") 

#define TEMP_SENSOR_0 1
#define TEMP_SENSOR_BED 1

#define THERMISTORHEATER_0 TEMP_SENSOR_0
#define BED_USES_THERMISTOR

#define TEMP_SENSOR_AD595_OFFSET 0.0
#define TEMP_SENSOR_AD595_GAIN   1.0

#define THERMISTORBED TEMP_SENSOR_BED

#define PIDTEMP
#define PID_FUNCTIONAL_RANGE 10
#define PID_MAX 255

#define K1 0.95
#define PID_dT ((16.0 * 8.0)/(F_CPU / 64.0 / 256.0))

#define  DEFAULT_Kp 22.2
#define  DEFAULT_Ki 1.08  
#define  DEFAULT_Kd 114  

#define HEATER_0_MINTEMP 5
#define HEATER_1_MINTEMP 5
#define HEATER_2_MINTEMP 5
#define BED_MINTEMP 5

#define HEATER_0_MAXTEMP 275
#define HEATER_1_MAXTEMP 275
#define HEATER_2_MAXTEMP 275
#define BED_MAXTEMP 150

#define MAX_BED_POWER 255

#define HEATER_0_RAW_HI_TEMP 0
#define HEATER_0_RAW_LO_TEMP 16383

#define HEATER_BED_RAW_LO_TEMP 16383
#define HEATER_BED_RAW_HI_TEMP 0

#define 	CLI()		__set_PRIMASK(1)
#define		SEI()		__set_PRIMASK(0)

#define 	CRITICAL_SECTION_START 	CLI() 
#define		CRITICAL_SECTION_END		SEI()
#endif
