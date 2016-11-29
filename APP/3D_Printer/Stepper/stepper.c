#include "stepper.h"
#include "defines.h"
#include "stm32f4xx_tim.h"
#include "planner.h"
#include "OS.h"
//#include "scarausart.h"
volatile long count_position[NUM_AXIS] = { 0, 0, 0, 0};
static long acceleration_time, deceleration_time;
char step_loops;
block_t *current_block;
static unsigned short acc_step_rate;
//unsigned short Prescaler_nominal;
static unsigned short Prescaler;
unsigned short Prescaler_nominal;



const uint16_t speed_lookuptable_fast[256][2] = {
   {62500, 54055}, {8445, 3917}, {4528, 1434}, {3094, 745}, {2349, 456}, {1893, 307}, {1586, 222}, {1364, 167},
   {1197, 131}, {1066, 105}, {961, 86}, {875, 72}, {803, 61}, {742, 53}, {689, 45}, {644, 40},
   {604, 35}, {569, 32}, {537, 28}, {509, 25}, {484, 23}, {461, 21}, {440, 19}, {421, 17},
   {404, 16}, {388, 15}, {373, 14}, {359, 13}, {346, 12}, {334, 11}, {323, 10}, {313, 10},
   {303, 9}, {294, 9}, {285, 8}, {277, 7}, {270, 8}, {262, 7}, {255, 6}, {249, 6},
   {243, 6}, {237, 6}, {231, 5}, {226, 5}, {221, 5}, {216, 5}, {211, 4}, {207, 5},
   {202, 4}, {198, 4}, {194, 4}, {190, 3}, {187, 4}, {183, 3}, {180, 3}, {177, 4},
   {173, 3}, {170, 3}, {167, 2}, {165, 3}, {162, 3}, {159, 2}, {157, 3}, {154, 2},
   {152, 3}, {149, 2}, {147, 2}, {145, 2}, {143, 2}, {141, 2}, {139, 2}, {137, 2},
   {135, 2}, {133, 2}, {131, 2}, {129, 1}, {128, 2}, {126, 2}, {124, 1}, {123, 2},
   {121, 1}, {120, 2}, {118, 1}, {117, 1}, {116, 2}, {114, 1}, {113, 1}, {112, 2},
   {110, 1}, {109, 1}, {108, 1}, {107, 2}, {105, 1}, {104, 1}, {103, 1}, {102, 1},
   {101, 1}, {100, 1}, {99, 1}, {98, 1}, {97, 1}, {96, 1}, {95, 1}, {94, 1},
   {93, 1}, {92, 1}, {91, 0}, {91, 1}, {90, 1}, {89, 1}, {88, 1}, {87, 0},
   {87, 1}, {86, 1}, {85, 1}, {84, 0}, {84, 1}, {83, 1}, {82, 1}, {81, 0},
   {81, 1}, {80, 1}, {79, 0}, {79, 1}, {78, 0}, {78, 1}, {77, 1}, {76, 0},
   {76, 1}, {75, 0}, {75, 1}, {74, 1}, {73, 0}, {73, 1}, {72, 0}, {72, 1},
   {71, 0}, {71, 1}, {70, 0}, {70, 1}, {69, 0}, {69, 1}, {68, 0}, {68, 1},
   {67, 0}, {67, 1}, {66, 0}, {66, 1}, {65, 0}, {65, 0}, {65, 1}, {64, 0},
   {64, 1}, {63, 0}, {63, 1}, {62, 0}, {62, 0}, {62, 1}, {61, 0}, {61, 1},
   {60, 0}, {60, 0}, {60, 1}, {59, 0}, {59, 0}, {59, 1}, {58, 0}, {58, 0},
   {58, 1}, {57, 0}, {57, 0}, {57, 1}, {56, 0}, {56, 0}, {56, 1}, {55, 0},
   {55, 0}, {55, 1}, {54, 0}, {54, 0}, {54, 1}, {53, 0}, {53, 0}, {53, 0},
   {53, 1}, {52, 0}, {52, 0}, {52, 1}, {51, 0}, {51, 0}, {51, 0}, {51, 1},
   {50, 0}, {50, 0}, {50, 0}, {50, 1}, {49, 0}, {49, 0}, {49, 0}, {49, 1},
   {48, 0}, {48, 0}, {48, 0}, {48, 1}, {47, 0}, {47, 0}, {47, 0}, {47, 1},
   {46, 0}, {46, 0}, {46, 0}, {46, 0}, {46, 1}, {45, 0}, {45, 0}, {45, 0},
   {45, 1}, {44, 0}, {44, 0}, {44, 0}, {44, 0}, {44, 1}, {43, 0}, {43, 0},
   {43, 0}, {43, 0}, {43, 1}, {42, 0}, {42, 0}, {42, 0}, {42, 0}, {42, 0},
   {42, 1}, {41, 0}, {41, 0}, {41, 0}, {41, 0}, {41, 0}, {41, 1}, {40, 0},
   {40, 0}, {40, 0}, {40, 0}, {40, 1}, {39, 0}, {39, 0}, {39, 0}, {39, 0},
   {39, 0}, {39, 0}, {39, 1}, {38, 0}, {38, 0}, {38, 0}, {38, 0}, {38, 0},
};

const uint16_t speed_lookuptable_slow[256][2] = {
   {62500, 10417}, {52083, 7441}, {44642, 5580}, {39062, 4340}, {34722, 3472}, {31250, 2841}, {28409, 2368}, {26041, 2003},
   {24038, 1717}, {22321, 1488}, {20833, 1302}, {19531, 1149}, {18382, 1021}, {17361, 914}, {16447, 822}, {15625, 745},
   {14880, 676}, {14204, 618}, {13586, 566}, {13020, 520}, {12500, 481}, {12019, 445}, {11574, 414}, {11160, 385},
   {10775, 359}, {10416, 336}, {10080, 315}, {9765, 296}, {9469, 278}, {9191, 263}, {8928, 248}, {8680, 235},
   {8445, 222}, {8223, 211}, {8012, 200}, {7812, 191}, {7621, 181}, {7440, 173}, {7267, 165}, {7102, 158},
   {6944, 151}, {6793, 145}, {6648, 138}, {6510, 133}, {6377, 127}, {6250, 123}, {6127, 118}, {6009, 113},
   {5896, 109}, {5787, 106}, {5681, 101}, {5580, 98}, {5482, 95}, {5387, 91}, {5296, 88}, {5208, 86},
   {5122, 82}, {5040, 80}, {4960, 78}, {4882, 75}, {4807, 73}, {4734, 70}, {4664, 69}, {4595, 67},
   {4528, 64}, {4464, 63}, {4401, 61}, {4340, 60}, {4280, 58}, {4222, 56}, {4166, 55}, {4111, 53},
   {4058, 52}, {4006, 51}, {3955, 49}, {3906, 48}, {3858, 48}, {3810, 45}, {3765, 45}, {3720, 44},
   {3676, 43}, {3633, 42}, {3591, 40}, {3551, 40}, {3511, 39}, {3472, 38}, {3434, 38}, {3396, 36},
   {3360, 36}, {3324, 35}, {3289, 34}, {3255, 34}, {3221, 33}, {3188, 32}, {3156, 31}, {3125, 31},
   {3094, 31}, {3063, 30}, {3033, 29}, {3004, 28}, {2976, 28}, {2948, 28}, {2920, 27}, {2893, 27},
   {2866, 26}, {2840, 25}, {2815, 25}, {2790, 25}, {2765, 24}, {2741, 24}, {2717, 24}, {2693, 23},
   {2670, 22}, {2648, 22}, {2626, 22}, {2604, 22}, {2582, 21}, {2561, 21}, {2540, 20}, {2520, 20},
   {2500, 20}, {2480, 20}, {2460, 19}, {2441, 19}, {2422, 19}, {2403, 18}, {2385, 18}, {2367, 18},
   {2349, 17}, {2332, 18}, {2314, 17}, {2297, 16}, {2281, 17}, {2264, 16}, {2248, 16}, {2232, 16},
   {2216, 16}, {2200, 15}, {2185, 15}, {2170, 15}, {2155, 15}, {2140, 15}, {2125, 14}, {2111, 14},
   {2097, 14}, {2083, 14}, {2069, 14}, {2055, 13}, {2042, 13}, {2029, 13}, {2016, 13}, {2003, 13},
   {1990, 13}, {1977, 12}, {1965, 12}, {1953, 13}, {1940, 11}, {1929, 12}, {1917, 12}, {1905, 12},
   {1893, 11}, {1882, 11}, {1871, 11}, {1860, 11}, {1849, 11}, {1838, 11}, {1827, 11}, {1816, 10},
   {1806, 11}, {1795, 10}, {1785, 10}, {1775, 10}, {1765, 10}, {1755, 10}, {1745, 9}, {1736, 10},
   {1726, 9}, {1717, 10}, {1707, 9}, {1698, 9}, {1689, 9}, {1680, 9}, {1671, 9}, {1662, 9},
   {1653, 9}, {1644, 8}, {1636, 9}, {1627, 8}, {1619, 9}, {1610, 8}, {1602, 8}, {1594, 8},
   {1586, 8}, {1578, 8}, {1570, 8}, {1562, 8}, {1554, 7}, {1547, 8}, {1539, 8}, {1531, 7},
   {1524, 8}, {1516, 7}, {1509, 7}, {1502, 7}, {1495, 7}, {1488, 7}, {1481, 7}, {1474, 7},
   {1467, 7}, {1460, 7}, {1453, 7}, {1446, 6}, {1440, 7}, {1433, 7}, {1426, 6}, {1420, 6},
   {1414, 7}, {1407, 6}, {1401, 6}, {1395, 7}, {1388, 6}, {1382, 6}, {1376, 6}, {1370, 6},
   {1364, 6}, {1358, 6}, {1352, 6}, {1346, 5}, {1341, 6}, {1335, 6}, {1329, 5}, {1324, 6},
   {1318, 5}, {1313, 6}, {1307, 5}, {1302, 6}, {1296, 5}, {1291, 5}, {1286, 6}, {1280, 5},
   {1275, 5}, {1270, 5}, {1265, 5}, {1260, 5}, {1255, 5}, {1250, 5}, {1245, 5}, {1240, 5},
   {1235, 5}, {1230, 5}, {1225, 5}, {1220, 5}, {1215, 4}, {1211, 5}, {1206, 5}, {1201, 5},
};

unsigned short calc_timer(unsigned short step_rate) 
{
		unsigned short timer;
		uint16_t table_address;
		unsigned char tmp_step_rate;
		unsigned short gain;
	
		if(step_rate > MAX_STEP_FREQUENCY) 
				step_rate = MAX_STEP_FREQUENCY;
		
		if(step_rate > 20000) 
		{ // If steprate > 20kHz >> step 4 times
				step_rate = (step_rate >> 2)&0x3fff;		//0x3fff = 16383 = 11 1111 1111 1111,  20000 >> 2 = 5000 = 0x1388 = 1 0011 1000 1000
				step_loops = 4;
		}
		else if(step_rate > 10000) 
		{ // If steprate > 10kHz >> step 2 times
				step_rate = (step_rate >> 1)&0x7fff;
				step_loops = 2;
		}
		else 
		{
				step_loops = 1;
		} 
  
		if(step_rate < (F_CPU / 500000)) 
				step_rate = (F_CPU/500000);
		
		step_rate -= (F_CPU/500000); // Correct for minimal speed
		
		if(step_rate >= (8 * 256))
		{ // higher step rate 
			table_address = speed_lookuptable_fast[(unsigned char)(step_rate >> 8)][0];
			gain = (unsigned short)speed_lookuptable_fast[(unsigned char)(step_rate >> 8)][1];
			tmp_step_rate = (step_rate & 0x00ff);
			MultiU16X8toH16(timer, tmp_step_rate, gain);
			timer = (unsigned short)speed_lookuptable_fast[(unsigned char)(step_rate >> 8)][0] - timer;
		}
		else { // lower step rates
//			table_address = (unsigned short)&speed_lookuptable_slow[0][0];
//			table_address += ((step_rate)>>1) & 0xfffc;
//			timer = (unsigned short)pgm_read_word_near(table_address);
			timer = (unsigned short)speed_lookuptable_fast[((step_rate) >> 1) & 0xfffc][0];
			timer -= (((unsigned short)speed_lookuptable_fast[((step_rate) >> 1) & 0xfffc][1] * (unsigned char)(step_rate & 0x0007))>>3);
		}
  if(timer < 100) 
	{ 
			timer = 100; 
//			MYSERIAL.print(MSG_STEPPER_TO_HIGH); 
//			MYSERIAL.println(step_rate); 
	}//(20kHz this should never happen)
  return timer;
}

void trapezoid_generator_reset(void) 
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		#ifdef ADVANCE
			advance = current_block->initial_advance;
			final_advance = current_block->final_advance;
			// Do E steps + advance steps
			e_steps[current_block->active_extruder] += ((advance >>8) - old_advance);
			old_advance = advance >>8;  
		#endif
	
		deceleration_time = 0;
		// step_rate to timer interval

		Prescaler_nominal = calc_timer(current_block->nominal_rate);

		acc_step_rate = current_block->initial_rate;
		acceleration_time = calc_timer(acc_step_rate);
		Prescaler = acceleration_time;   
		
		TIM_TimeBaseStructure.TIM_Period = 19999;
		TIM_TimeBaseStructure.TIM_Prescaler = Prescaler;
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//0
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
}

void critical_section_start(void)
{
		TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
		TIM_Cmd(TIM4, DISABLE);
}

void critical_section_end(void)
{
		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM4, ENABLE);		
}

void st_set_position(const long x, const long y, const long z, const long e)
{	   
//		usart_putstrln("start st set");
//		CRITICAL_SECTION_START;
//		critical_section_start();
		
		CPU_SR_ALLOC();
		CPU_CRITICAL_ENTER();
		OSIntEnter();
		CPU_CRITICAL_EXIT();
		count_position[X_AXIS] = x;
		count_position[Y_AXIS] = y;
		count_position[Z_AXIS] = z;
		count_position[E_AXIS] = e;
		OSIntExit();
//		CRITICAL_SECTION_END; 
//		critical_section_end();
//		usart_putstrln("end st set");
}

void st_set_e_position(const long e)
{
//  CRITICAL_SECTION_START;
		CPU_SR_ALLOC();
		CPU_CRITICAL_ENTER();
		OSIntEnter();
		CPU_CRITICAL_EXIT();
		count_position[E_AXIS] = e;
		OSIntExit();
//  CRITICAL_SECTION_END;
}
