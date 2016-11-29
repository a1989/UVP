#include "temperature.h"
#include "defines.h"
#include "setpins.h"

#define ARRAY_BY_EXTRUDERS(v1, v2, v3) 	{ v1 }

#define _TT_NAME(_N) 	temptable_ ## _N
#define TT_NAME(_N) 	_TT_NAME(_N)

#define HEATER_0_TEMPTABLE 	TT_NAME(THERMISTORHEATER_0)
#define HEATER_0_TEMPTABLE_LEN (sizeof(HEATER_0_TEMPTABLE)/sizeof(*HEATER_0_TEMPTABLE))

#define BEDTEMPTABLE 	TT_NAME(THERMISTORBED)
#define BEDTEMPTABLE_LEN 	(sizeof(BEDTEMPTABLE) / sizeof(*BEDTEMPTABLE))

static int minttemp[EXTRUDERS] = ARRAY_BY_EXTRUDERS( 0, 0, 0 );
static int maxttemp[EXTRUDERS] = ARRAY_BY_EXTRUDERS( 16383, 16383, 16383 );

static float pid_error[EXTRUDERS];
static float temp_iState_min[EXTRUDERS];
  static float temp_iState_max[EXTRUDERS];
int target_temperature[EXTRUDERS] = { 0 };
static bool pid_reset[EXTRUDERS];
static float temp_iState[EXTRUDERS] = { 0 };
static float temp_dState[EXTRUDERS] = { 0 };
static float pTerm[EXTRUDERS];
static float iTerm[EXTRUDERS];
static float dTerm[EXTRUDERS];

float Kp = DEFAULT_Kp;
float Ki = (DEFAULT_Ki*PID_dT);
float Kd = (DEFAULT_Kd/PID_dT);

unsigned char soft_pwm[EXTRUDERS];
unsigned char soft_pwm_bed;

int target_temperature_bed = 0;

extern __IO uint16_t uhADC1ConvertedValue;
extern __IO uint16_t uhADC2ConvertedValue;

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define OVERSAMPLENR 16
#if (THERMISTORHEATER_0 == 1) || (THERMISTORHEATER_1 == 1)  || (THERMISTORHEATER_2 == 1) || (THERMISTORBED == 1) //100k bed thermistor

int current_temperature_raw[EXTRUDERS] = { 0 };

const short temptable_1[][2]= {
{       23*OVERSAMPLENR ,       300     },
{       25*OVERSAMPLENR ,       295     },
{       27*OVERSAMPLENR ,       290     },
{       28*OVERSAMPLENR ,       285     },
{       31*OVERSAMPLENR ,       280     },
{       33*OVERSAMPLENR ,       275     },
{       35*OVERSAMPLENR ,       270     },
{       38*OVERSAMPLENR ,       265     },
{       41*OVERSAMPLENR ,       260     },
{       44*OVERSAMPLENR ,       255     },
{       48*OVERSAMPLENR ,       250     },
{       52*OVERSAMPLENR ,       245     },
{       56*OVERSAMPLENR ,       240     },
{       61*OVERSAMPLENR ,       235     },
{       66*OVERSAMPLENR ,       230     },
{       71*OVERSAMPLENR ,       225     },
{       78*OVERSAMPLENR ,       220     },
{       84*OVERSAMPLENR ,       215     },
{       92*OVERSAMPLENR ,       210     },
{       100*OVERSAMPLENR        ,       205     },
{       109*OVERSAMPLENR        ,       200     },
{       120*OVERSAMPLENR        ,       195     },
{       131*OVERSAMPLENR        ,       190     },
{       143*OVERSAMPLENR        ,       185     },
{       156*OVERSAMPLENR        ,       180     },
{       171*OVERSAMPLENR        ,       175     },
{       187*OVERSAMPLENR        ,       170     },
{       205*OVERSAMPLENR        ,       165     },
{       224*OVERSAMPLENR        ,       160     },
{       245*OVERSAMPLENR        ,       155     },
{       268*OVERSAMPLENR        ,       150     },
{       293*OVERSAMPLENR        ,       145     },
{       320*OVERSAMPLENR        ,       140     },
{       348*OVERSAMPLENR        ,       135     },
{       379*OVERSAMPLENR        ,       130     },
{       411*OVERSAMPLENR        ,       125     },
{       445*OVERSAMPLENR        ,       120     },
{       480*OVERSAMPLENR        ,       115     },
{       516*OVERSAMPLENR        ,       110     },
{       553*OVERSAMPLENR        ,       105     },
{       591*OVERSAMPLENR        ,       100     },
{       628*OVERSAMPLENR        ,       95      },
{       665*OVERSAMPLENR        ,       90      },
{       702*OVERSAMPLENR        ,       85      },
{       737*OVERSAMPLENR        ,       80      },
{       770*OVERSAMPLENR        ,       75      },
{       801*OVERSAMPLENR        ,       70      },
{       830*OVERSAMPLENR        ,       65      },
{       857*OVERSAMPLENR        ,       60      },
{       881*OVERSAMPLENR        ,       55      },
{       903*OVERSAMPLENR        ,       50      },
{       922*OVERSAMPLENR        ,       45      },
{       939*OVERSAMPLENR        ,       40      },
{       954*OVERSAMPLENR        ,       35      },
{       966*OVERSAMPLENR        ,       30      },
{       977*OVERSAMPLENR        ,       25      },
{       985*OVERSAMPLENR        ,       20      },
{       993*OVERSAMPLENR        ,       15      },
{       999*OVERSAMPLENR        ,       10      },
{       1004*OVERSAMPLENR       ,       5       },
{       1008*OVERSAMPLENR       ,       0       } //safety
};
#endif

volatile bool temp_meas_ready = false;
static void *heater_ttbl_map[EXTRUDERS] = ARRAY_BY_EXTRUDERS( (void *)HEATER_0_TEMPTABLE, (void *)HEATER_1_TEMPTABLE, (void *)HEATER_2_TEMPTABLE );
static uint8_t heater_ttbllen_map[EXTRUDERS] = ARRAY_BY_EXTRUDERS( HEATER_0_TEMPTABLE_LEN, HEATER_1_TEMPTABLE_LEN, HEATER_2_TEMPTABLE_LEN );

float current_temperature[EXTRUDERS] = { 0 };
float current_temperature_bed = 0;
int current_temperature_bed_raw = 0;

int maxttemp_raw[EXTRUDERS] = ARRAY_BY_EXTRUDERS( HEATER_0_RAW_HI_TEMP , HEATER_1_RAW_HI_TEMP , HEATER_2_RAW_HI_TEMP );
int minttemp_raw[EXTRUDERS] = ARRAY_BY_EXTRUDERS( HEATER_0_RAW_LO_TEMP , HEATER_1_RAW_LO_TEMP , HEATER_2_RAW_LO_TEMP );
int bed_maxttemp_raw = HEATER_BED_RAW_HI_TEMP;

void setTargetBed(const float celsius) 
{  
		target_temperature_bed = celsius;
};

float degHotend(uint8_t extruder)
{
		return current_temperature[extruder];
//		return 100;
//		return current_temperature[0];
}

float degTargetHotend(uint8_t extruder)
{  
		return target_temperature[extruder];
}

float degBed(void) 
{
		return current_temperature_bed;
}

float degTargetBed(void) 
{   
		return target_temperature_bed;
}

static float analog2temp(int raw, uint8_t e)
{
//  if(e >= EXTRUDERS)
//  {
//      SERIAL_ERROR_START;
//      SERIAL_ERROR((int)e);
//      SERIAL_ERRORLNPGM(" - Invalid extruder number !");
//      kill();
//  } 
		#ifdef HEATER_0_USES_MAX6675
    if (e == 0)
    {
      return 0.25 * raw;
    }
		#endif

  if(heater_ttbl_map[e] != NULL)
  {
    float celsius = 0;
    uint8_t i;
    short (*tt)[][2] = (short (*)[][2])(heater_ttbl_map[e]);

    for (i = 1; i < heater_ttbllen_map[e]; i++)
    {
      if (((*tt)[i][0]) > raw)
      {
        celsius = ((*tt)[i-1][1]) + 
          (raw - ((*tt)[i-1][0])) * 
          (float)(((*tt)[i][1]) - ((*tt)[i-1][1])) /
          (float)(((*tt)[i][0]) - ((*tt)[i-1][0]));
        break;
      }
    }

    // Overflow: Set to last value in the table
    if (i == heater_ttbllen_map[e]) celsius = ((*tt)[i-1][1]);

    return celsius;
  }
  return ((raw * ((5.0 * 100.0) / 1024.0) / OVERSAMPLENR) * TEMP_SENSOR_AD595_GAIN) + TEMP_SENSOR_AD595_OFFSET;
}

static float analog2tempBed(int raw) 
{
  #ifdef BED_USES_THERMISTOR
    float celsius = 0;
    unsigned char i;

    for (i = 1; i < BEDTEMPTABLE_LEN; i++)
    {
      if ((BEDTEMPTABLE[i][0]) > raw)
      {
        celsius  = (BEDTEMPTABLE[i-1][1]) + 
          (raw - (BEDTEMPTABLE[i-1][0])) * 
          (float)((BEDTEMPTABLE[i][1]) - (BEDTEMPTABLE[i-1][1])) /
          (float)((BEDTEMPTABLE[i][0]) - (BEDTEMPTABLE[i-1][0]));
        break;
      }
    }

    // Overflow: Set to last value in the table
    if (i == BEDTEMPTABLE_LEN) celsius = (BEDTEMPTABLE[i-1][1]);

    return celsius;
  #elif defined BED_USES_AD595
    return ((raw * ((5.0 * 100.0) / 1024.0) / OVERSAMPLENR) * TEMP_SENSOR_AD595_GAIN) + TEMP_SENSOR_AD595_OFFSET;
  #else
    return 0;
  #endif
}

static void updateTemperaturesFromRawValues(void)
{
		uint8_t e;
    for(e = 0; e < EXTRUDERS; e++)
    {
        current_temperature[e] = analog2temp(current_temperature_raw[e], e);
    }
    current_temperature_bed = analog2tempBed(current_temperature_bed_raw);

    //Reset the watchdog after we know we have a temperature measurement.
//    watchdog_reset();

//    CRITICAL_SECTION_START;
    temp_meas_ready = false;
//    CRITICAL_SECTION_END;
}

int getHeaterPower(int heater) 
{
		if (heater < 0)
				return soft_pwm_bed;
		return soft_pwm[heater];
}

void get_adc_raw_value(void)
{
		current_temperature_raw[0] = (int)uhADC1ConvertedValue * 3.8;
		current_temperature_bed_raw = (int)uhADC2ConvertedValue * 3.8;
}

void manage_heater(void)
{
		  float pid_input;
			float pid_output;
			int e;
	
//			TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
//			TIM_Cmd(TIM3, ENABLE);
//			if(temp_meas_ready != true)   //better readability
//			return; 

			get_adc_raw_value();
			updateTemperaturesFromRawValues();		//current_temperature[e], current_temperature_bed, temp_meas_ready = false

			for(e = 0; e < EXTRUDERS; e++) 
			{
					#ifdef PIDTEMP
					pid_input = current_temperature[e];

							#ifndef PID_OPENLOOP
							pid_error[e] = target_temperature[e] - pid_input;
						
							if(pid_error[e] > PID_FUNCTIONAL_RANGE) 
							{
									pid_output = PID_MAX;
									pid_reset[e] = true;
							}
					
							else if(pid_error[e] < -PID_FUNCTIONAL_RANGE)
							{
									pid_output = 0;
									pid_reset[e] = true;
							}
							
							else 
							{
									if(pid_reset[e] == true) 
									{
											temp_iState[e] = 0.0;
											pid_reset[e] = false;
									}
									pTerm[e] = Kp * pid_error[e];
									temp_iState[e] += pid_error[e];
									temp_iState[e] = constrain(temp_iState[e], temp_iState_min[e], temp_iState_max[e]);
									iTerm[e] = Ki * temp_iState[e];

									//K1 defined in Configuration.h in the PID settings
									#define K2 (1.0 - K1)
									dTerm[e] = (Kd * (pid_input - temp_dState[e]))*K2 + (K1 * dTerm[e]);
									temp_dState[e] = pid_input;

									pid_output = constrain(pTerm[e] + iTerm[e] - dTerm[e], 0, PID_MAX);
							}
							#else 
									pid_output = constrain(target_temperature[e], 0, PID_MAX);
							#endif //PID_OPENLOOP not define

					#else /* PID off */
					pid_output = 0;
					if(current_temperature[e] < target_temperature[e]) 
					{
							pid_output = PID_MAX;
					}
					#endif  //PIDTEMP

					// Check if temperature is within the correct range
					if((current_temperature[e] > minttemp[e]) && (current_temperature[e] < maxttemp[e])) 
					{
							soft_pwm[e] = (int)pid_output >> 1;
					}
					else 
					{
							soft_pwm[e] = 0;
					}

					#ifdef WATCH_TEMP_PERIOD
					if(watchmillis[e] && millis() - watchmillis[e] > WATCH_TEMP_PERIOD)
					{
							if(degHotend(e) < watch_start_temp[e] + WATCH_TEMP_INCREASE)
							{
									setTargetHotend(0, e);
									LCD_MESSAGEPGM("Heating failed");
									SERIAL_ECHO_START;
									SERIAL_ECHOLN("Heating failed");
							}else{
									watchmillis[e] = 0;
							}
					}
					#endif

			} // End extruder for loop
			
			#if TEMP_SENSOR_BED != 0
			
			#ifdef PIDTEMPBED
					pid_input = current_temperature_bed;

					#ifndef PID_OPENLOOP
							pid_error_bed = target_temperature_bed - pid_input;
							pTerm_bed = bedKp * pid_error_bed;
							temp_iState_bed += pid_error_bed;
							temp_iState_bed = constrain(temp_iState_bed, temp_iState_min_bed, temp_iState_max_bed);
							iTerm_bed = bedKi * temp_iState_bed;

							//K1 defined in Configuration.h in the PID settings
							#define K2 (1.0-K1)
									dTerm_bed= (bedKd * (pid_input - temp_dState_bed))*K2 + (K1 * dTerm_bed);
									temp_dState_bed = pid_input;

									pid_output = constrain(pTerm_bed + iTerm_bed - dTerm_bed, 0, MAX_BED_POWER);

					#else 
							pid_output = constrain(target_temperature_bed, 0, MAX_BED_POWER);
					#endif //PID_OPENLOOP

	  if((current_temperature_bed > BED_MINTEMP) && (current_temperature_bed < BED_MAXTEMP)) 
	  {
	    soft_pwm_bed = (int)pid_output >> 1;
	  }
	  else {
	    soft_pwm_bed = 0;
	  }

    #elif !defined(BED_LIMIT_SWITCHING)
      // Check if temperature is within the correct range
      if((current_temperature_bed > BED_MINTEMP) && (current_temperature_bed < BED_MAXTEMP))
      {
        if(current_temperature_bed >= target_temperature_bed)
        {
          soft_pwm_bed = 0;
        }
        else 
        {
          soft_pwm_bed = MAX_BED_POWER >> 1;
        }
      }
      else
      {
        soft_pwm_bed = 0;
        WRITE_Pin(HEATER_BED_PIN_PORT, HEATER_BED_PIN, LOW);
      }
    #else //#ifdef BED_LIMIT_SWITCHING
      // Check if temperature is within the correct band
      if((current_temperature_bed > BED_MINTEMP) && (current_temperature_bed < BED_MAXTEMP))
      {
        if(current_temperature_bed > target_temperature_bed + BED_HYSTERESIS)
        {
          soft_pwm_bed = 0;
        }
        else if(current_temperature_bed <= target_temperature_bed - BED_HYSTERESIS)
        {
          soft_pwm_bed = MAX_BED_POWER >> 1;
        }
      }
      else
      {
        soft_pwm_bed = 0;
        WRITE(HEATER_BED_PIN,LOW);
      }
    #endif
  #endif
}

void max_temp_error(uint8_t e)
{
		
}

void min_temp_error(uint8_t e)
{
	
}

void bed_max_temp_error(void)
{
		
}

void setTargetHotend(const float celsius, uint8_t extruder) 
{  
		target_temperature[extruder] = celsius;
}