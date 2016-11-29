#include "function.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "planner.h"
#include "stm32f4xx.h"

float destination[NUM_AXIS] = {0.0, 0.0, 0.0, 0.0};
float current_position[NUM_AXIS] = {0.0, 0.0, 0.0, 0.0};
float delta[3] = {0.0, 0.0, 0.0};
float min_pos[3] = { X_MIN_POS, Y_MIN_POS, Z_MIN_POS };
float max_pos[3] = { X_MAX_POS, Y_MAX_POS, Z_MAX_POS };
float axis_scaling[3] = {1, 1, 0};
char axis_codes[NUM_AXIS] = {'X', 'Y', 'Z', 'E'};
static bool home_all_axis;
static bool home_XY;
volatile long endstops_trigsteps[3]={0,0,0};

char cmdbuffer[BUFSIZE][MAX_CMD_SIZE];

int home_dir[3] = {1, 1, -1}; 	//X, Y means linkage1_motor, linkage2_motor
//float axis_steps_per_unit[4] = {100, 100, 20, 100};
const float axis_steps_per_unit[4] = {53.33, 53.33, 800, 120};    //X, Y -> degree, Z,E -> mm
//const float axis_steps_per_unit[4] = {62.74, 62.74, 800, 120};
//float axis_steps_per_unit[4] = {89.99, 89.99, 800, 240};
//float axis_steps_per_unit[4] = {300, 300, 5, 100};
float max_feedrate[4] = {100, 100, 1000, 20};
float add_homeing[3]={0,0,0};

uint32_t buflen = 0;
uint32_t bufindr = 0;
uint32_t bufindw = 0;

char serial_count = 0;
char cmdbuf[96];

bool comment_mode = false;
float homing_feedrate[] = HOMING_FEEDRATE;
float feedrate = 1500.0;
float next_feedrate;
float saved_feedrate;
int saved_feedmultiply;
int feedmultiply = 100;

bool axis_relative_mode[] = {false, false, false, false};
bool relative_mode = false;
bool SoftEndsEnabled = true; 

bool Stopped = false;

uint8_t active_extruder = 0;

uint32_t Stopped_gcode_LaseN = 0;

uint8_t which_menu_display = 0;
uint8_t previous_menu = 0;

bool fromsd[BUFSIZE];
char serial_char;
char *strchr_pointer;
uint32_t gcode_N = 0;
uint32_t gcode_LastN = 0;

float SCARA_C2, SCARA_S2, SCARA_K1, SCARA_K2, SCARA_theta, SCARA_psi;

float SCARA_MNab1, SCARA_MNab2, SCARA_aMN, SCARA_theta1, SCARA_theta2;

//GUI_PID_STATE Touch_stat;

extern long position[4];
extern int x_step;
extern int y_step;

extern __IO uint16_t uhADC1ConvertedValue;
extern int current_temperature_raw[EXTRUDERS];
static uint8_t tmp_extruder;

float checkbuf[3];


extern float current_temperature[EXTRUDERS];
extern int target_temperature[EXTRUDERS];

char debug_buf[96];
char debug_last[96];

float code_value(void)
{
		return (strtod(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL));
}

bool code_seen(char code)
{
		strchr_pointer = strchr(cmdbuffer[bufindr], code);
		return (strchr_pointer != NULL);
}

void get_coordinates()
{
		uint8_t i;
		bool seen[4] = {false, false, false, false};
		for(i = 0; i < NUM_AXIS; i++)
		{
				if(code_seen(axis_codes[i]))
				{
						destination[i] = (float)code_value() + 
															(axis_relative_mode[i] || relative_mode) * current_position[i];
						seen[i] = true;
				}
				else
						destination[i] = current_position[i];
		}
		if(code_seen('F'))
		{
				next_feedrate = code_value();
				if(next_feedrate > 0.0)
						feedrate = next_feedrate;
		}
}

void calculate_delta(float cartesian[3])
{
		float SCARA_pos[2];

		SCARA_pos[X_AXIS] = SCARA_offset_x - cartesian[X_AXIS] * axis_scaling[X_AXIS];
		SCARA_pos[Y_AXIS] = SCARA_offset_y - cartesian[Y_AXIS] * axis_scaling[Y_AXIS];		

//		SCARA_MNab1 = pow(SCARA_pos[X_AXIS], 2) + 
//								pow(SCARA_pos[Y_AXIS], 2) + 
//								pow(Linkage_1 / 1000, 2) - 
//								pow(Linkage_2 / 1000, 2);
	
		SCARA_MNab1 = pow(SCARA_pos[X_AXIS], 2) + 
								pow(SCARA_pos[Y_AXIS], 2) + 
								pow(Linkage_1, 2) - 
								pow(Linkage_2, 2);
								
//		SCARA_MNab2 = pow(SCARA_pos[X_AXIS], 2) + 
//								pow(SCARA_pos[Y_AXIS], 2) - 
//								pow(Linkage_1 / 1000, 2) - 
//								pow(Linkage_2 / 1000, 2);	
	
		SCARA_MNab2 = pow(SCARA_pos[X_AXIS], 2) + 
								pow(SCARA_pos[Y_AXIS], 2) - 
								pow(Linkage_1, 2) - 
								pow(Linkage_2, 2);		
								
		SCARA_aMN = 2 * Linkage_1 * ( sqrt( pow(SCARA_pos[X_AXIS], 2) + pow(SCARA_pos[Y_AXIS], 2)) );
		
	  SCARA_theta1 = atan2(SCARA_pos[Y_AXIS], SCARA_pos[X_AXIS]) + acos(SCARA_MNab1 / SCARA_aMN); 
		SCARA_theta2 = acos(SCARA_MNab2 / (2 * Linkage_1 * Linkage_2 * 1));

		delta[X_AXIS] = (SCARA_theta1 * SCARA_RAD2DEG + 180);
//		delta[X_AXIS] = SCARA_theta1 * SCARA_RAD2DEG;

		delta[Y_AXIS] = (SCARA_theta2 - SCARA_theta1) * SCARA_RAD2DEG;   //SCARA_theta2  * SCARA_RAD2DEG - SCARA_theta1 * SCARA_RAD2DEG;

		delta[Z_AXIS] = cartesian[Z_AXIS];
}

void clamp_to_software_endstops(float target[3])
{
		if(min_software_endstops)
		{
				if(target[X_AXIS] < min_pos[X_AXIS])
						target[X_AXIS] = min_pos[X_AXIS];
				if(target[Y_AXIS] < min_pos[Y_AXIS])
						target[Y_AXIS] = min_pos[Y_AXIS];
				if(target[Z_AXIS] < min_pos[Z_AXIS])
						target[Z_AXIS] = min_pos[Z_AXIS];
		}
		
		if(max_software_endstops)
		{
				if(target[X_AXIS] > max_pos[X_AXIS])
						target[X_AXIS] = max_pos[X_AXIS];
				if(target[Y_AXIS] > max_pos[Y_AXIS])
						target[Y_AXIS] = max_pos[Y_AXIS];
				if(target[Z_AXIS] > max_pos[Z_AXIS])
						target[Z_AXIS] = max_pos[Z_AXIS];
		}
}

void ENABLE_STEPPER_DRIVER_INTERRUPT(void)
{
		
}

void prepare_move(void)
{
		float difference[NUM_AXIS];
		uint32_t i, j;
		float seconds;
		int32_t steps;
		float fraction;
		float cartesian_mm;

		if(SoftEndsEnabled)
		{
				clamp_to_software_endstops(destination);
		}
		
		for(i = 0; i <= NUM_AXIS; i++)
		{
				difference[i] = destination[i] - current_position[i];
		}
		
		for(i = 0; i < 3; i++)
		{
				checkbuf[i] = difference[i];
		}
		
		cartesian_mm = sqrt(sq(difference[X_AXIS]) + sq(difference[Y_AXIS]) + sq(difference[Z_AXIS]));
		
		
		if(cartesian_mm < 0.0)
		{
				cartesian_mm = abs(difference[E_AXIS]);
		}
		if(cartesian_mm < 0.0)
				return;
		seconds = 6000 * cartesian_mm / feedrate / feedmultiply;
		steps = max(1, (DELTA_SEGMENT_PER_SECOND * seconds));
		
		for(i = 1; i <= steps; i++)
		{
				fraction = (float)i / (float)steps;
			
				for(j = 0; j < NUM_AXIS; j++)
				{
						destination[j] = current_position[j] + difference[j] * fraction;
				}

				calculate_delta(destination);
				
				
				
				plan_buffer_line(delta[X_AXIS], delta[Y_AXIS], delta[Z_AXIS], destination[E_AXIS], feedrate * feedmultiply / 60 / 100.0,active_extruder);
		}

		for(i = 0; i < NUM_AXIS; i++)
		{
				current_position[i] = destination[i];
		}
}

void st_synchronize()
{
    while( blocks_queued()) 
		{
			manage_heater();
			Update_PrintingWindow();
		}
}

static void axis_is_at_home(int axis)
{ 
		if(axis == 2)
		{
				current_position[axis] = 0;
		}
		else if(axis == 0)
		{
				current_position[axis] = 214;
		}
		else
		{
				current_position[axis] = 214;
		}
		min_pos[axis] = 0;
		if(axis == X_AXIS)
		{
				max_pos[axis] = 220;
		}
		else if(axis == Y_AXIS)
		{
				max_pos[axis] = 220;
		}
		else
		{
				max_pos[axis] = 300;
		}
}

void endstops_hit_on_purpose()
{
		endstop_x_hit = false;
		endstop_y_hit = false;
		endstop_z_hit = false;
}

static void HOMEAXIS(int axis) 
{
		int i;
		current_position[axis] = 0;

    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

    destination[axis] = 10 * Z_MAX_LENGTH * home_dir[axis];

    feedrate = homing_feedrate[axis];
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);

    st_synchronize();
	
		if(axis == Z_AXIS)
		{
				current_position[axis] = 0;
		}
		else if(axis == X_AXIS)
		{
				current_position[axis] = 214;
		}
		else
		{
				current_position[axis] = 214;
		}

    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
		if(axis == 2)
		{
				destination[axis] = -home_retract_mm * home_dir[axis];
		}
		else if(axis == 1)
		{
				destination[axis] = Y_MAX_POS - home_retract_mm * home_dir[axis] * 0.05;
		}
		else
		{
				destination[axis] = X_MAX_POS - home_retract_mm * home_dir[axis] * 0.05;
		}
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();
		
		if(axis == 2)
		{
				destination[axis] = 2 * home_retract_mm * home_dir[axis];
		}
		else if(axis == 1)
		{
				destination[axis] = Y_MAX_POS + home_retract_mm * home_dir[axis];
		}
		else
		{
				destination[axis] = X_MAX_POS + home_retract_mm * home_dir[axis] * 5;
//				destination[axis] = 2 * home_retract_mm * home_dir[axis];
		}

    feedrate = homing_feedrate[axis] / 2 ; 
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();
		
    axis_is_at_home(axis);					
    destination[axis] = current_position[axis];
    feedrate = 0.0;
    endstops_hit_on_purpose();
		
//		trans = true;
}

void enable_endstops(bool check)
{
		check_endstops = check;
}

void serial_delay(uint32_t Count)
{
		while(Count--);
}

void ClearToSend()
{

  #ifdef SDSUPPORT
  if(fromsd[bufindr])
    return;
  #endif //SDSUPPORT

}

void FlushSerialRequestResend(void)
{
//		buffer_flush();
		ClearToSend();
}

void delayfortest(uint32_t Count)
{
		while(Count--);
}

void test_move(void)
{
		int i;
		WRITE_Pin(X_ENABLE_PIN_PORT, X_ENABLE_PIN, LOW);
		WRITE_Pin(X_DIR_PIN_PORT, X_DIR_PIN, HIGH);
//		for(i = 0; i < 200; i++)
		while(1)
		{
				WRITE_Pin(X_STEP_PIN_PORT, X_STEP_PIN, LOW);
				delayfortest(0x5ff);
				WRITE_Pin(X_STEP_PIN_PORT, X_STEP_PIN, HIGH);
				delayfortest(0x5ff);
		}
}

bool setTargetedHotend(int code){
  tmp_extruder = active_extruder;
  if(code_seen('T')) 
	{
    tmp_extruder = code_value();
    if(tmp_extruder >= EXTRUDERS) {

      switch(code){
        case 104:
          break;
        case 105:
          break;
        case 109:
          break;
      }
      return true;
    }
  }
  return false;
}

void Update_PrintingWindow(void)
{
			WM_HWIN hItem;
	
			hItem = WM_GetDialogItem(hWin, (GUI_ID_USER + 0x01));
			PROGBAR_SetValue(hItem, current_temperature[0]);
}


bool isHeatingHotend(uint8_t extruder)
{  
  return target_temperature[extruder] > current_temperature[extruder];
}

bool isCoolingHotend(uint8_t extruder) 
{  
  return target_temperature[extruder] < current_temperature[extruder];
}

void process_commands(void)
{	
//		usart_putstr("enter process_commands!\n");
		int8_t i;
		bool target_direction;
//		usart_putstr("enter process_commands!\n");
		if(code_seen('G'))
		{
				switch((int) code_value())
				{
						case 0:
						case 1:
								get_coordinates();
								prepare_move();

								break;
						case 28:
								saved_feedrate = feedrate;
								saved_feedmultiply = feedmultiply;
								feedmultiply = 100;
								enable_endstops(true);

								for(i = 0; i < NUM_AXIS; i++)
								{
										destination[i] = current_position[i];
								}
								feedrate = 0;
								home_XY = (code_seen(axis_codes[0]) || (code_seen(axis_codes[1])) );
								home_all_axis = !( (code_seen(axis_codes[0])) || (code_seen(axis_codes[1])) || (code_seen(axis_codes[2])) ) 
								|| ( (code_seen(axis_codes[0])) && (code_seen(axis_codes[1])) && (code_seen(axis_codes[2])) );
								
								if((home_XY || home_all_axis) || (code_seen(axis_codes[X_AXIS])))
								{
										HOMEAXIS(X_AXIS);
								}
								if((home_XY || home_all_axis) || (code_seen(axis_codes[Y_AXIS])))
								{
										HOMEAXIS(Y_AXIS);
								}
								if((home_all_axis) || (code_seen(axis_codes[Z_AXIS])))
								{
										HOMEAXIS(Z_AXIS);
								}
								
								calculate_delta(current_position);
								plan_set_position(delta[X_AXIS], delta[Y_AXIS], delta[Z_AXIS], current_position[E_AXIS]);
								feedrate = saved_feedrate;
								feedmultiply = saved_feedmultiply;
								endstops_hit_on_purpose();

								break;
						case 90:
								relative_mode = false;
								break;
						case 91:
								relative_mode = true;
								break;
						case 92:
								if(!code_seen(axis_codes[E_AXIS]))
									st_synchronize();
								for(i = 0; i < NUM_AXIS; i++) 
								{
										if(code_seen(axis_codes[i])) 
										{ 
												if(i == E_AXIS) 
												{
														current_position[i] = code_value();  
														plan_set_e_position(current_position[E_AXIS]);
												}
												else 
												{
														if (i == X_AXIS || i == Y_AXIS) 
														{
																current_position[i] = code_value();  
														}
														else 
														{
																current_position[i] = code_value() + add_homeing[i];  
														}    
														plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
												}
										}
								}
								break;
						default:
								break;
				}
		}//code 'G'
		
		else if(code_seen('M'))
		{
				switch((int)code_value())
				{
						case 0:
								break;
						case 1:
								break;
						case 104:
							  if(setTargetedHotend(104))
										break;
								if (code_seen('S')) 
										setTargetHotend(code_value(), tmp_extruder);
//										setWatch();
								break;
						case 105:
								break;
						case 140:
							  if (code_seen('S')) 
										setTargetBed(code_value());
								break;
						case 115:
								break;
						case 109:
								if(setTargetedHotend(109))
								{
										break;
								}
								
//								#ifdef AUTOTEMP
//										autotemp_enabled=false;
//								#endif
								
								if (code_seen('S')) 
								{
										setTargetHotend(code_value(), tmp_extruder);
//										CooldownNoWait = true;
								}
								
								target_direction = isHeatingHotend(tmp_extruder);
								
								while(target_direction ? (isHeatingHotend(tmp_extruder)) : (isCoolingHotend(tmp_extruder)))
								{
										manage_heater();
										Update_PrintingWindow();
								}
										
								break;
						case 107:
							break;
						case 117:
							break;
						default:
							break;
				}
		}	
		ClearToSend();
}

bool card_eof(void)
{		
		return (f_eof(&fil));
}

void get_cmd(void)
{
	SD_Opreations();
}

void NVIC_Config(void)
{
		NVIC_InitTypeDef   NVIC_InitStructure;
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}

void TIM_Config(void)
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		TIM_TimeBaseStructure.TIM_Period = 99;
		TIM_TimeBaseStructure.TIM_Prescaler = 89;
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//0
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
		TIM_ClearFlag(TIM4,TIM_FLAG_Update);

		TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);

		TIM_Cmd(TIM4, DISABLE);
}

void TIM_Init(void)
{		
		NVIC_Config();
		TIM_Config();
}

void scara_settings(void)
{
		IO_Init();
		tp_init();
		plan_init();
		st_init();
		TIM_Init();
}

void Move_Axis_Home(int Axis)
{
		switch(Axis)
		{
				case 0:
					HOMEAXIS(X_AXIS);
					HOMEAXIS(Y_AXIS);
					break;
				case 1:
					HOMEAXIS(Z_AXIS);
					break;
				case 2:
					HOMEAXIS(X_AXIS);
					HOMEAXIS(Y_AXIS);
					HOMEAXIS(Z_AXIS);
					break;
				default:
					break;
		}
}



void Move_Axis_mm(int Axis, int direction)
{
		current_position[Axis] += direction;
	
		switch(Axis)
		{
				case 0:
						if (current_position[Axis] > X_MAX_POS)
								current_position[Axis] = X_MAX_POS;
						if (current_position[Axis] < X_MIN_POS)
								current_position[Axis] = X_MIN_POS;
								break;
				case 1:
						if (current_position[Axis] > Y_MAX_POS)
								current_position[Axis] = Y_MAX_POS;
						if (current_position[Axis] < Y_MIN_POS)
								current_position[Axis] = Y_MIN_POS;
								break;
				case 2:
						if (current_position[Axis] > Z_MAX_POS)
								current_position[Axis] = Z_MAX_POS;
						if (current_position[Axis] < Z_MIN_POS)
								current_position[Axis] = Z_MIN_POS;
						break;
				default:
						break;
		}

	prepare_move();	
}


void UV_operation_task(void)
{

}
