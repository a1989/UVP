/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    13-April-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "includes.h"

#define CHECK_ENDSTOPS  if(check_endstops)
#define USART1_SR_BASE            (0x40011000)
#define USART1_SR                *(__IO uint16_t * )(USART1_SR_BASE)

static unsigned char out_bits; 
extern block_t *current_block;
static long counter_x,
						counter_y,
						counter_z,
						counter_e;
volatile static unsigned long step_events_completed;
volatile signed char count_direction[NUM_AXIS] = { 1, 1, 1, 1};


extern char step_loops;

static unsigned short acc_step_rate;
static long acceleration_time, deceleration_time;

const bool X_ENDSTOPS_INVERTING = true; // set to true to invert the logic of the endstops. 
const bool Y_ENDSTOPS_INVERTING = true; // set to true to invert the logic of the endstops. 
const bool Z_ENDSTOPS_INVERTING = true; // set to true to invert the logic of the endstops. 


extern unsigned short Prescaler_nominal;
extern struct ring_buffer rx_buffer;
int cmd_buffer_count = 0;
int test_step = 0;
int test_it = 0;

extern unsigned char soft_pwm[EXTRUDERS];
extern unsigned char soft_pwm_bed;

extern __IO uint16_t uhADC1ConvertedValue;
extern volatile bool temp_meas_ready;
extern int current_temperature_raw[EXTRUDERS];
extern int current_temperature_bed_raw;
extern int maxttemp_raw[EXTRUDERS];
extern int minttemp_raw[EXTRUDERS];
extern int bed_maxttemp_raw;
extern int target_temperature_bed;
extern volatile long endstops_trigsteps[3];
extern volatile long count_position[NUM_AXIS];
/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup IOToggle
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
////		OS_TimeMS ++;
//}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void IT_delay(volatile int Count)
{
		while(Count--);
}


void TIM3_IRQHandler(void)
{

		static unsigned char temp_count = 0;
		static unsigned long raw_temp_0_value = 0;
		static unsigned long raw_temp_1_value = 0;
		static unsigned long raw_temp_2_value = 0;
		static unsigned long raw_temp_bed_value = 0;
		static unsigned char temp_state = 0;
		static unsigned char pwm_count = 1;
		static unsigned char soft_pwm_0;
		static unsigned char soft_pwm_b;
	
		if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
//		usart_putstrln("here tim3 intterupt");	
		if(pwm_count == 0)
		{
				soft_pwm_0 = soft_pwm[0];
			
				if(soft_pwm_0 > 0) 
						WRITE_Pin(HEATER_END_PIN_PORT, HEATER_END_PIN,1);
				#if EXTRUDERS > 1
						soft_pwm_1 = soft_pwm[1];
						if(soft_pwm_1 > 0) 
								WRITE(HEATER_1_PIN,1);
				#endif
						
				#if EXTRUDERS > 2
						soft_pwm_2 = soft_pwm[2];
						if(soft_pwm_2 > 0) 
								WRITE(HEATER_2_PIN,1);
				#endif
						
				soft_pwm_b = soft_pwm_bed;
				if(soft_pwm_b > 0) 
						WRITE_Pin(HEATER_BED_PIN_PORT, HEATER_BED_PIN, 1);
			}
		
		  if(soft_pwm_0 <= pwm_count) 
					WRITE_Pin(HEATER_END_PIN_PORT, HEATER_END_PIN, 0);
			
			#if EXTRUDERS > 1
			if(soft_pwm_1 <= pwm_count) 
					WRITE(HEATER_1_PIN,0);
			#endif
			
			#if EXTRUDERS > 2
			if(soft_pwm_2 <= pwm_count) 
					WRITE(HEATER_2_PIN,0);
			#endif

			if(soft_pwm_b <= pwm_count) 
					WRITE_Pin(HEATER_BED_PIN_PORT, HEATER_BED_PIN, 0);
			
			pwm_count++;
			pwm_count &= 0x7f;
			
			switch(temp_state) 
			{
					case 0: // Prepare TEMP_0
							temp_state = 1;
							break;
					case 1: // Measure TEMP_0
							raw_temp_0_value = uhADC1ConvertedValue * 3.8;
							temp_state = 2;
							break;
					case 2: // Prepare TEMP_BED
							temp_state = 3;
							break;
					case 3: // Measure TEMP_BED
//							raw_temp_bed_value += ADC;
							temp_state = 4;
							break;
					case 4: // Prepare TEMP_1
							temp_state = 5;
							break;
					case 5: // Measure TEMP_1
							temp_state = 6;
							break;
					case 6: // Prepare TEMP_2
							temp_state = 7;
							break;
					case 7: // Measure TEMP_2
							temp_state = 0;
							temp_count++;
							break;
//    		default:
//      			SERIAL_ERROR_START;
//      			SERIAL_ERRORLNPGM("Temp measurement error!");
//      			break;
			}
			
			if(temp_count >= 16) // 8 ms * 16 = 128ms.
			{
					if (!temp_meas_ready) //Only update the raw values if they have been read. Else we could be updating them during reading.
					{
							current_temperature_raw[0] = raw_temp_0_value;
							
							#if EXTRUDERS > 1
										current_temperature_raw[1] = raw_temp_1_value;
							#endif
							
							#if EXTRUDERS > 2
										current_temperature_raw[2] = raw_temp_2_value;
							#endif
										current_temperature_bed_raw = raw_temp_bed_value;
					}
    
					temp_meas_ready = true;
					temp_count = 0;
					raw_temp_0_value = 0;
					raw_temp_1_value = 0;
					raw_temp_2_value = 0;
					raw_temp_bed_value = 0;

					#if HEATER_0_RAW_LO_TEMP > HEATER_0_RAW_HI_TEMP
							if(current_temperature_raw[0] <= maxttemp_raw[0]) 
					#else
							if(current_temperature_raw[0] >= maxttemp_raw[0]) 
					#endif
							{
									max_temp_error(0);
							}
							
					#if HEATER_0_RAW_LO_TEMP > HEATER_0_RAW_HI_TEMP
							if(current_temperature_raw[0] >= minttemp_raw[0]) 
					#else
							if(current_temperature_raw[0] <= minttemp_raw[0]) 
					#endif
							{
									min_temp_error(0);
							}
							
//					#if EXTRUDERS > 1
//					#if HEATER_1_RAW_LO_TEMP > HEATER_1_RAW_HI_TEMP
//							if(current_temperature_raw[1] <= maxttemp_raw[1]) {
//					#else
//							if(current_temperature_raw[1] >= maxttemp_raw[1]) {
//					#endif
//									max_temp_error(1);
//							}
//					#if HEATER_1_RAW_LO_TEMP > HEATER_1_RAW_HI_TEMP
//							if(current_temperature_raw[1] >= minttemp_raw[1]) {
//					#else
//							if(current_temperature_raw[1] <= minttemp_raw[1]) {
//					#endif
//									min_temp_error(1);
//							}
//					#endif
//					#if EXTRUDERS > 2
//					#if HEATER_2_RAW_LO_TEMP > HEATER_2_RAW_HI_TEMP
//							if(current_temperature_raw[2] <= maxttemp_raw[2]) {
//					#else
//							if(current_temperature_raw[2] >= maxttemp_raw[2]) {
//					#endif
//									max_temp_error(2);
//							}
//					#if HEATER_2_RAW_LO_TEMP > HEATER_2_RAW_HI_TEMP
//							if(current_temperature_raw[2] >= minttemp_raw[2]) {
//					#else
//							if(current_temperature_raw[2] <= minttemp_raw[2]) {
//					#endif
//									min_temp_error(2);
//							}
//					#endif
  
  /* No bed MINTEMP error? */
				#if defined(BED_MAXTEMP) && (TEMP_SENSOR_BED != 0)
						#if HEATER_BED_RAW_LO_TEMP > HEATER_BED_RAW_HI_TEMP
								if(current_temperature_bed_raw <= bed_maxttemp_raw) 
						#else
								if(current_temperature_bed_raw >= bed_maxttemp_raw) 
						#endif
								{
										target_temperature_bed = 0;
										bed_max_temp_error();
								}
				#endif
								}
	}
}

void TIM4_IRQHandler(void)
{
		bool x_min_endstop;
//		bool x_max_endstop;
		bool y_min_endstop;
		unsigned short timer;
		unsigned short step_rate;
		int8_t i;
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		
//		CPU_SR_ALLOC();
//		CPU_CRITICAL_ENTER();
//		OSIntEnter();
//		CPU_CRITICAL_EXIT();
		
//		GUI_DispStringAt("enter the interrupt!", 10, 20);
//		usart_putstrln("enter the interrupt!");
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)  //检测制定的中断是否发生
		{
				test_it++;
				TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //清除中断处理位。
				
				if(current_block == NULL)
				{
						current_block = plan_get_current_block();
						if(current_block != NULL)
						{
								current_block -> busy = true;
								trapezoid_generator_reset();
								counter_x = -(current_block->step_event_count >> 1);
								counter_y = counter_x;
								counter_z = counter_x;
								counter_e = counter_x;
								step_events_completed = 0; 
						}
						else		//1kHz
						{
//								TIM_TimeBaseStructure.TIM_Period = 1999;
//								TIM_TimeBaseStructure.TIM_Prescaler = 89;
								TIM_TimeBaseStructure.TIM_Period = 19999;
								TIM_TimeBaseStructure.TIM_Prescaler = 8;
								TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//0
								TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
								TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
						}
				}//current_block == NULL
				
				if(current_block != NULL)
				{
//						usart_putstrln("here current_block != NULL");
						out_bits = current_block -> direction_bits;
						
						if( (out_bits & (1 << X_AXIS) ) != 0)
						{
								#ifndef COREXY
										WRITE_Pin(X_DIR_PIN_PORT, X_DIR_PIN, INVERT_X_DIR);
								#endif
								count_direction[X_AXIS] = -1;
/*								
								CHECK_ENDSTOPS
								{
										x_min_endstop = (READ_Pin(X_MIN_PIN_PORT, X_MIN_PIN) != X_ENDSTOPS_INVERTING);
										
										if(x_min_endstop && old_x_min_endstop && (current_block->steps_x > 0))
										{
												endstops_trigsteps[X_AXIS] = count_position[X_AXIS];
												endstop_x_hit = true;
												step_events_completed = current_block->step_event_count;
										}
										old_x_min_endstop = x_min_endstop;
								}//CHECK_ENDSTOPS
*/
						}//(out_bits & (1 << X_AXIS) ) != 0
						else
						{
								#if !defined COREXY  //NOT COREXY
										WRITE_Pin(X_DIR_PIN_PORT, X_DIR_PIN,!INVERT_X_DIR);
								#endif
								count_direction[X_AXIS] = 1;

								CHECK_ENDSTOPS 
								{
										bool x_max_endstop=(READ_Pin(X_MAX_PIN_PORT, X_MAX_PIN) != X_ENDSTOPS_INVERTING);
										if(x_max_endstop && old_x_max_endstop && (current_block -> steps_x > 0))
										{
												endstops_trigsteps[X_AXIS] = count_position[X_AXIS];
												endstop_x_hit=true;
												step_events_completed = current_block->step_event_count;
										}
										old_x_max_endstop = x_max_endstop;
       
								}//CHECK_ENDSTOPS 
						}
						
						if ((out_bits & (1<<Y_AXIS)) != 0) 
						{   // -direction
								#if !defined COREXY  //NOT COREXY
										WRITE_Pin(Y_DIR_PIN_PORT, Y_DIR_PIN,INVERT_Y_DIR);
								#endif
								count_direction[Y_AXIS]=-1;
								//count_direction[Y_AXIS]=1;
//								CHECK_ENDSTOPS
//								{
//									//	#if Y_MAX_PIN > -1
//										bool y_max_endstop=(READ_Pin(Y_MAX_PIN_PORT, Y_MAX_PIN) != Y_ENDSTOPS_INVERTING);
//										if(y_max_endstop && old_y_max_endstop && (current_block->steps_y > 0))
//										{
//												endstops_trigsteps[Y_AXIS] = count_position[Y_AXIS];
//												endstop_y_hit=true;
//												step_events_completed = current_block->step_event_count;
//										}
//										old_y_max_endstop = y_max_endstop;
//									//	#endif
//								}
/*							
								CHECK_ENDSTOPS
								{
										y_min_endstop=(READ_Pin(Y_MIN_PIN_PORT, Y_MIN_PIN) != Y_ENDSTOPS_INVERTING);
										if(y_min_endstop && old_y_min_endstop && (current_block->steps_y > 0)) 
										{
												endstops_trigsteps[Y_AXIS] = count_position[Y_AXIS];
												endstop_y_hit=true;
												step_events_completed = current_block->step_event_count;
										}
										old_y_min_endstop = y_min_endstop;
								}
*/
						}
						else 
						{ // +direction
								#if !defined COREXY  //NOT COREXY
										WRITE_Pin(Y_DIR_PIN_PORT, Y_DIR_PIN,!INVERT_Y_DIR);
								#endif
								count_direction[Y_AXIS]=1;
								CHECK_ENDSTOPS
								{
									//	#if Y_MAX_PIN > -1
										bool y_max_endstop=(READ_Pin(Y_MAX_PIN_PORT, Y_MAX_PIN) != Y_ENDSTOPS_INVERTING);
//										usart_putstr("y");
//										USART_Put_I2A(y_max_endstop);
										if(y_max_endstop && old_y_max_endstop && (current_block->steps_y > 0))
										{
												endstops_trigsteps[Y_AXIS] = count_position[Y_AXIS];
												endstop_y_hit=true;
												step_events_completed = current_block->step_event_count;
										}
										old_y_max_endstop = y_max_endstop;
									//	#endif
								}
						}
		
						if ((out_bits & (1<<Z_AXIS)) != 0) 
						{   // -direction
								WRITE_Pin(Z_DIR_PIN_PORT, Z_DIR_PIN, INVERT_Z_DIR);
      
								#ifdef Z_DUAL_STEPPER_DRIVERS
										WRITE(Z2_DIR_PIN,INVERT_Z_DIR);
								#endif
      
								count_direction[Z_AXIS]=-1;
								CHECK_ENDSTOPS
								{
//										#if Z_MIN_PIN > -1
										bool z_min_endstop=(READ_Pin(Z_MIN_PIN_PORT, Z_MIN_PIN) != Z_ENDSTOPS_INVERTING);
//										usart_putstr("z_min_endstop = ");
//										USART_Put_I2A(z_min_endstop);
										if(z_min_endstop && old_z_min_endstop && (current_block->steps_z > 0)) 
										{
//												usart_putstrln("hit z");
												endstops_trigsteps[Z_AXIS] = count_position[Z_AXIS];
												endstop_z_hit=true;
												step_events_completed = current_block->step_event_count;
										}
								old_z_min_endstop = z_min_endstop;
//								#endif
								}
						}
						else 
						{ // +direction
								WRITE_Pin(Z_DIR_PIN_PORT, Z_DIR_PIN,!INVERT_Z_DIR);

								#ifdef Z_DUAL_STEPPER_DRIVERS
										WRITE(Z2_DIR_PIN,!INVERT_Z_DIR);
								#endif

								count_direction[Z_AXIS]=1;

								CHECK_ENDSTOPS
								{
										#if Z_MAX_PIN > -1
										bool z_max_endstop = READ(Z_MAX_PIN) != Z_ENDSTOPS_INVERTING);
//										usart_putstr("z_max_endstop = ");
//										USART_Put_I2A(z_max_endstop);
										if(z_max_endstop && old_z_max_endstop && (current_block->steps_z > 0)) 
										{
												endstops_trigsteps[Z_AXIS] = count_position[Z_AXIS];
												endstop_z_hit = true;
												step_events_completed = current_block->step_event_count;
										}
										old_z_max_endstop = z_max_endstop;
										#endif
								}
						}
		
						#ifndef ADVANCE
						if ((out_bits & (1 << E_AXIS)) != 0) 
						{  // -direction
//								REV_E_DIR();
								WRITE_Pin(E0_DIR_PIN_PORT, E0_DIR_PIN, INVERT_E0_DIR);
								count_direction[E_AXIS]=-1;
						}
						else 
						{
							// +direction
//								NORM_E_DIR();
								WRITE_Pin(E0_DIR_PIN_PORT, E0_DIR_PIN, !INVERT_E0_DIR);
								count_direction[E_AXIS] = 1;
						}
						#endif //!ADVANCE
//						usart_putstr("step_loops = ");
//						USART_Put_I2A(step_loops);
						for(i = 0; i < step_loops; i++) 
						{ // Take multiple steps per interrupt (For high speed moves) 	
//								usart_putstrln("enter the for loop");
//								usart_putstr("step_loops = ");
//							USART_Put_I2A(step_loops);
								#if !defined COREXY      
										counter_x += current_block->steps_x;
										if (counter_x > 0) 
										{
//												usart_putstrln("here write the x step pin");
												WRITE_Pin(X_STEP_PIN_PORT, X_STEP_PIN, !INVERT_X_STEP_PIN);
												IT_delay(0x4fff);
												//IT_delay(0x3fff);
												counter_x -= current_block->step_event_count;
												count_position[X_AXIS]+=count_direction[X_AXIS];   
												WRITE_Pin(X_STEP_PIN_PORT, X_STEP_PIN, INVERT_X_STEP_PIN);
										}
							
										counter_y += current_block->steps_y;
										if (counter_y > 0) 
										{
//												usart_putstrln("here write the y step pin");
												WRITE_Pin(Y_STEP_PIN_PORT, Y_STEP_PIN, !INVERT_Y_STEP_PIN);
												IT_delay(0x4fff);
												//IT_delay(0x3fff);
												counter_y -= current_block->step_event_count; 
												count_position[Y_AXIS]+=count_direction[Y_AXIS]; 
												WRITE_Pin(Y_STEP_PIN_PORT, Y_STEP_PIN, INVERT_Y_STEP_PIN);
										}
								#endif
				
								counter_z += current_block->steps_z;
//								usart_putstrln("current_block->steps_z=");
//								USART_Put_F2A(current_block->steps_z);
								if (counter_z > 0) 
								{
//										usart_putstrln("here write the z step pin");
										WRITE_Pin(Z_STEP_PIN_PORT, Z_STEP_PIN, !INVERT_Z_STEP_PIN);
										IT_delay(0xcf0);
										#ifdef Z_DUAL_STEPPER_DRIVERS
												WRITE_Pin(Z2_STEP_PIN, !INVERT_Z_STEP_PIN);
										#endif
										
										counter_z -= current_block->step_event_count;
										count_position[Z_AXIS] += count_direction[Z_AXIS];
										WRITE_Pin(Z_STEP_PIN_PORT,Z_STEP_PIN, INVERT_Z_STEP_PIN);
										
										#ifdef Z_DUAL_STEPPER_DRIVERS
												WRITE(Z2_STEP_PIN, INVERT_Z_STEP_PIN);
										#endif
//										test_step++;
										
								}
			
								#ifndef ADVANCE
										counter_e += current_block->steps_e;
										if (counter_e > 0) 
										{
													WRITE_Pin(E0_STEP_PIN_PORT, E0_STEP_PIN, !INVERT_Z_DIR);
													IT_delay(0xcf0);
													counter_e -= current_block->step_event_count;
													count_position[E_AXIS]+=count_direction[E_AXIS];
													WRITE_Pin(E0_STEP_PIN_PORT, E0_STEP_PIN, INVERT_Z_DIR);
										}
								#endif //!ADVANCE
								step_events_completed += 1;  
								if(step_events_completed >= current_block->step_event_count) break;
						}//for loop
						

						if (step_events_completed <= (unsigned long int)current_block->accelerate_until) 
						{						
								MultiU24X24toH16(acc_step_rate, acceleration_time, current_block->acceleration_rate);
								acc_step_rate += current_block -> initial_rate;
								
								// upper limit
								if(acc_step_rate > current_block -> nominal_rate)
									acc_step_rate = current_block -> nominal_rate;

								// step_rate to timer interval
								timer = calc_timer(acc_step_rate);
								
//								OCR1A = timer;
								TIM_TimeBaseStructure.TIM_Period = 1999;
								TIM_TimeBaseStructure.TIM_Prescaler = timer;
								TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//0
								TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
								TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
								
								acceleration_time += timer;
						}//step_events_completed <= (unsigned long int)current_block->accelerate_until
						
						else if (step_events_completed > (unsigned long int)current_block->decelerate_after) 
						{   
								MultiU24X24toH16(step_rate, deceleration_time, current_block->acceleration_rate);
								
								if(step_rate > acc_step_rate) 
								{ // Check step_rate stays positive
										step_rate = current_block->final_rate;
								}
								else 
								{
										step_rate = acc_step_rate - step_rate; // Decelerate from aceleration end point.
								}

								// lower limit
								if(step_rate < current_block->final_rate)
									step_rate = current_block->final_rate;

								// step_rate to timer interval
								timer = calc_timer(step_rate);
//							usart_putstr("timer = ");
//							USART_Put_I2A(timer);							
								//OCR1A = timer;
								TIM_TimeBaseStructure.TIM_Period = 1999;
								TIM_TimeBaseStructure.TIM_Prescaler = timer;
								TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//0
								TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
								TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
								
								deceleration_time += timer;
						}//else if
						
						else 
						{
								//OCR1A = OCR1A_nominal;
								TIM_TimeBaseStructure.TIM_Period = 1999;
								TIM_TimeBaseStructure.TIM_Prescaler = Prescaler_nominal;
								TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//0
								TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
								TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
						}
						
						if (step_events_completed >= current_block->step_event_count) 
						{
								current_block = NULL;
								plan_discard_current_block();
								TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
								TIM_Cmd(TIM4, DISABLE);
						}   
				}//current_block != NULL
		}
		
//		OSIntExit();
}

//extern void SysTick_ISR(void);	/* 声明调用外部的函数 */
//void SysTick_Handler(void)
//{
//	SysTick_ISR();	/* 这个函数在bsp_timer.c中 */	
//}

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
