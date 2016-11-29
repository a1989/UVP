#ifndef __PLANNER_H
#define __PLANNER_H

#include "stm32f4xx.h"
#include "stdbool.h"

#define sq(x) ((x)*(x))
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

typedef struct{
	
		long steps_x, steps_y, steps_z, steps_e;
		unsigned long step_event_count;
		long accelerate_until;
		long decelerate_after;
		long acceleration_rate;
		unsigned char direction_bits;
		unsigned char active_extruder;
		
		float nominal_speed;
		float entry_speed;
		float max_entry_speed;
		float millimeters;
		float acceleration;
		unsigned char recalculate_flag;
		unsigned char nominal_length_flag;
		
		unsigned long nominal_rate;
		unsigned long initial_rate;
		unsigned long final_rate;
		unsigned long acceleration_st;
		unsigned long fan_speed;
		volatile char busy;
}block_t;

#define BLOCK_BUFFER_SIZE 16

 

bool blocks_queued(void);
void plan_init(void);
void plan_set_position(const float x, const float y, const float z, const float e);
void plan_buffer_line(float x, float y, float z, float e, float feed_rate, uint8_t extruder);
block_t *plan_get_current_block(void); 
void plan_discard_current_block(void);
void plan_set_e_position(const float e);

#endif
