#include "planner.h"
#include "defines.h"
#include "math.h"
#include "stdio.h"
#include "stepper.h"
#include "setpins.h"
//#include "GUI.h"
//#include "scarausart.h"
#include "temperature.h"
#include "OS.h"

unsigned long axis_steps_per_sqr_second[NUM_AXIS];
long position[4];
extern float axis_steps_per_unit[4];
extern float max_feedrate[4];
const unsigned int dropsegments = 5;
int extrudemultiply = 100;
int fanSpeed=0;

float mintravelfeedrate;
float minimumfeedrate;

float retract_acceleration;
float acceleration;
float max_xy_jerk;
float max_z_jerk;
float max_e_jerk;

volatile unsigned char block_buffer_head;
volatile unsigned char block_buffer_tail;

int x_step = 0;
int y_step = 0;

block_t block_buffer[BLOCK_BUFFER_SIZE];

#define ENABLE_STEPPER_DRIVER_INTERRUPT()  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE)

extern void Update_PrintingWindow(void);

bool blocks_queued(void) 
{
  if (block_buffer_head == block_buffer_tail) 
	{ 
    return false; 
  }
  else
    return true;
}

void plan_init(void)
{
  block_buffer_head = 0;
  block_buffer_tail = 0;
  memset(position, 0, sizeof(position)); // clear position
  previous_speed[0] = 0.0;
  previous_speed[1] = 0.0;
  previous_speed[2] = 0.0;
  previous_speed[3] = 0.0;
  previous_nominal_speed = 0.0;
}

void plan_set_position(const float x, const float y, const float z, const float e)
{
		position[X_AXIS] = lround(x*axis_steps_per_unit[X_AXIS]);
//		position[X_AXIS] = lround(203.94 * axis_steps_per_unit[X_AXIS]);
		position[Y_AXIS] = lround(y*axis_steps_per_unit[Y_AXIS]);
//		position[Y_AXIS] = lround(130.47 * axis_steps_per_unit[Y_AXIS]);
		position[Z_AXIS] = lround(z*axis_steps_per_unit[Z_AXIS]);     
		position[E_AXIS] = lround(e*axis_steps_per_unit[E_AXIS]);  
		st_set_position(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[E_AXIS]);
		previous_nominal_speed = 0.0; // Resets planner junction speeds. Assumes start from rest.
		previous_speed[0] = 0.0;
		previous_speed[1] = 0.0;
		previous_speed[2] = 0.0;
		previous_speed[3] = 0.0;
}

void plan_set_e_position(const float e)
{
  position[E_AXIS] = lround(e*axis_steps_per_unit[E_AXIS]);  
  st_set_e_position(position[E_AXIS]);
}

block_t *plan_get_current_block(void) 
{
		block_t *block;
		
		if (block_buffer_head == block_buffer_tail) 
		{ 
			return(NULL); 
		}
		
		block = &block_buffer[block_buffer_tail];
		block->busy = true;
		return(block);
}

void plan_discard_current_block(void)  
{
		if (block_buffer_head != block_buffer_tail) 
		{
			block_buffer_tail = (block_buffer_tail + 1) & (BLOCK_BUFFER_SIZE - 1);  
		}
}

// Gets the current block. Returns NULL if buffer empty

void st_wake_up(void) 
{
		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM4, ENABLE);
}

int8_t next_block_index(int8_t block_index)
{
		block_index++;
		if(block_index == BLOCK_BUFFER_SIZE)
		{
				block_index = 0;
		}
		return block_index;
}

float max_allowable_speed(float acceleration, float target_velocity, float distance) 
{
		return  sqrt(target_velocity * target_velocity - 2 * acceleration * distance);
}

void planner_forward_pass_kernel(block_t *previous, block_t *current, block_t *next) 
{
		if(!previous) 
		{ 
				return; 
		}

  // If the previous block is an acceleration block, but it is not long enough to complete the
  // full speed change within the block, we need to adjust the entry speed accordingly. Entry
  // speeds have already been reset, maximized, and reverse planned by reverse planner.
  // If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
  if (!previous->nominal_length_flag) {
    if (previous->entry_speed < current->entry_speed) {
      double entry_speed = min( current->entry_speed,
      max_allowable_speed(-previous->acceleration,previous->entry_speed,previous->millimeters) );

      // Check for junction speed change
      if (current->entry_speed != entry_speed) {
        current->entry_speed = entry_speed;
        current->recalculate_flag = true;
      }
    }
  }
}

void planner_forward_pass() {
  uint8_t block_index = block_buffer_tail;
  block_t *block[3] = { 
    NULL, NULL, NULL   };

  while(block_index != block_buffer_head) {
    block[0] = block[1];
    block[1] = block[2];
    block[2] = &block_buffer[block_index];
    planner_forward_pass_kernel(block[0],block[1],block[2]);
    block_index = next_block_index(block_index);
  }
  planner_forward_pass_kernel(block[1], block[2], NULL);
}

float estimate_acceleration_distance(float initial_rate, float target_rate, float acceleration)
{
  if (acceleration!=0) {
    return((target_rate*target_rate-initial_rate*initial_rate)/
      (2.0*acceleration));
  }
  else {
    return 0.0;  // acceleration was 0, set acceleration distance to 0
  }
}

float intersection_distance(float initial_rate, float final_rate, float acceleration, float distance)
{
  if (acceleration!=0) {
    return((2.0*acceleration*distance-initial_rate*initial_rate+final_rate*final_rate)/
      (4.0*acceleration) );
  }
  else {
    return 0.0;  // acceleration was 0, set intersection distance to 0
  }
}

void calculate_trapezoid_for_block(block_t *block, float entry_factor, float exit_factor) 
{
		unsigned long initial_rate = ceil(block->nominal_rate*entry_factor); // (step/min)
		unsigned long final_rate = ceil(block->nominal_rate*exit_factor); // (step/min)
		long acceleration;
		int32_t accelerate_steps;
		int32_t decelerate_steps;
		int32_t plateau_steps;
		CPU_SR_ALLOC();
  // Limit minimal step rate (Otherwise the timer will overflow.)
		if(initial_rate <120) 
		{
				initial_rate=120; 
		}
		if(final_rate < 120) 
		{
				final_rate=120;  
		}

		acceleration = block->acceleration_st;
		accelerate_steps = ceil(estimate_acceleration_distance(block->initial_rate, block->nominal_rate, acceleration));
		decelerate_steps = floor(estimate_acceleration_distance(block->nominal_rate, block->final_rate, -acceleration));

  // Calculate the size of Plateau of Nominal Rate.
		plateau_steps = block->step_event_count-accelerate_steps-decelerate_steps;

  // Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
  // have to use intersection_distance() to calculate when to abort acceleration and start braking
  // in order to reach the final_rate exactly at the end of this block.
  if (plateau_steps < 0) {
    accelerate_steps = ceil(intersection_distance(block->initial_rate, block->final_rate, acceleration, block->step_event_count));
    accelerate_steps = max(accelerate_steps,0); // Check limits due to numerical round-off
    accelerate_steps = min((uint32_t)accelerate_steps,block->step_event_count);//(We can cast here to unsigned, because the above line ensures that we are above zero)
    plateau_steps = 0;
  }

#ifdef ADVANCE
  volatile long initial_advance = block->advance*entry_factor*entry_factor; 
  volatile long final_advance = block->advance*exit_factor*exit_factor;
#endif // ADVANCE

  // block->accelerate_until = accelerate_steps;
  // block->decelerate_after = accelerate_steps+plateau_steps;
//  CRITICAL_SECTION_START;  // Fill variables used by the stepper in a critical section
			
		CPU_CRITICAL_ENTER();
		OSIntEnter();
		CPU_CRITICAL_EXIT();

  if(block->busy == false) { // Don't update variables if block is busy.
    block->accelerate_until = accelerate_steps;
    block->decelerate_after = accelerate_steps+plateau_steps;
    block->initial_rate = initial_rate;
    block->final_rate = final_rate;
#ifdef ADVANCE
    block->initial_advance = initial_advance;
    block->final_advance = final_advance;
#endif //ADVANCE
  }
//  CRITICAL_SECTION_END;	
	OSIntExit();
}                 

static int8_t prev_block_index(int8_t block_index) {
  if (block_index == 0) { 
    block_index = BLOCK_BUFFER_SIZE; 
  }
  block_index--;
  return(block_index);
}

void planner_recalculate_trapezoids() {
  int8_t block_index = block_buffer_tail;
  block_t *current;
  block_t *next = NULL;

  while(block_index != block_buffer_head) {
    current = next;
    next = &block_buffer[block_index];
    if (current) {
      // Recalculate if current block entry or exit junction speed has changed.
      if (current->recalculate_flag || next->recalculate_flag) {
        // NOTE: Entry and exit factors always > 0 by all previous logic operations.
        calculate_trapezoid_for_block(current, current->entry_speed/current->nominal_speed,
        next->entry_speed/current->nominal_speed);
        current->recalculate_flag = false; // Reset current only to ensure next trapezoid is computed
      }
    }
    block_index = next_block_index( block_index );
  }
  // Last/newest block in buffer. Exit speed is set with MINIMUM_PLANNER_SPEED. Always recalculated.
  if(next != NULL) {
    calculate_trapezoid_for_block(next, next->entry_speed/next->nominal_speed,
    MINIMUM_PLANNER_SPEED/next->nominal_speed);
    next->recalculate_flag = false;
  }
}

void planner_reverse_pass_kernel(block_t *previous, block_t *current, block_t *next) {
  if(!current) { 
    return; 
  }

  if (next) {
    // If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
    // If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
    // check for maximum allowable speed reductions to ensure maximum possible planned speed.
    if (current->entry_speed != current->max_entry_speed) {

      // If nominal length true, max junction speed is guaranteed to be reached. Only compute
      // for max allowable speed if block is decelerating and nominal length is false.
      if ((!current->nominal_length_flag) && (current->max_entry_speed > next->entry_speed)) {
        current->entry_speed = min( current->max_entry_speed,
        max_allowable_speed(-current->acceleration,next->entry_speed,current->millimeters));
      } 
      else {
        current->entry_speed = current->max_entry_speed;
      }
      current->recalculate_flag = true;

    }
  } // Skip last block. Already initialized and set for recalculation.
}



void planner_reverse_pass(void)
{
		uint8_t block_index = block_buffer_head;
		unsigned char tail;
		block_t *block[3] = {NULL, NULL, NULL};
	//CRITICAL_SECTION_START;
	
		CPU_SR_ALLOC();
		CPU_CRITICAL_ENTER();
		OSIntEnter();
		CPU_CRITICAL_EXIT();	
	
		tail = block_buffer_tail;
	
		OSIntExit();
  //CRITICAL_SECTION_END
		

  if(((block_buffer_head-tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1)) > 3) {
    block_index = (block_buffer_head - 3) & (BLOCK_BUFFER_SIZE - 1);

    while(block_index != tail) { 
      block_index = prev_block_index(block_index); 
      block[2]= block[1];
      block[1]= block[0];
      block[0] = &block_buffer[block_index];
      planner_reverse_pass_kernel(block[0], block[1], block[2]);
    }
  }
}

void planner_recalculate(void) 
{   
  planner_reverse_pass();
  planner_forward_pass();
  planner_recalculate_trapezoids();
}

void plan_buffer_line(float x, float y, float z, float e, float feed_rate, uint8_t extruder)
{
		
		long target[4];
		int next_buffer_head = next_block_index(block_buffer_head);
		block_t *block = NULL;
		float delta_mm[4];
		float inverse_millimeters;
		float inverse_second;
		int moves_queued;
		float current_speed[4];
		float speed_factor = 1.0;
		float steps_per_mm;
		float vmax_junction;
		float vmax_junction_factor;
		float safe_speed;
		float jerk;
		double v_allowable;
		uint8_t i;
	
		while(block_buffer_tail == next_buffer_head)
		{
				manage_heater();
				Update_PrintingWindow();				
		}
//		GUI_DispStringAt("start planner!", 10, 200);
//		usart_putstrln("start planner");
		
		target[X_AXIS] = lround(x * axis_steps_per_unit[X_AXIS]);
		target[Y_AXIS] = lround(y * axis_steps_per_unit[Y_AXIS]);
		target[Z_AXIS] = lround(z * axis_steps_per_unit[Z_AXIS]);
		target[E_AXIS] = lround(e * axis_steps_per_unit[E_AXIS]);
		
//		usart_putstrln("target[X_AXIS]");
//		USART_Put_I2A(target[X_AXIS]);
//		usart_putstrln("target[Y_AXIS]");
//		USART_Put_I2A(target[Y_AXIS]);
//		
//		usart_putstrln("target[Z_AXIS]");
//		USART_Put_F2A(target[Z_AXIS]);
//		usart_putstrln("position[X_AXIS]");
//		USART_Put_I2A(position[X_AXIS]);
//		
//		usart_putstrln("position[Y_AXIS]");
//		USART_Put_I2A(position[Y_AXIS]);
//		GUI_DispDecAt(target[X_AXIS], 10, 240, 3);
		block = &block_buffer[block_buffer_head];
		block -> busy = false;
		block -> steps_x = labs(target[X_AXIS] - position[X_AXIS]);
		block -> steps_y = labs(target[Y_AXIS] - position[Y_AXIS]);
//		block -> steps_y = labs(position[Y_AXIS] - target[Y_AXIS]);
		block -> steps_z = labs(target[Z_AXIS] - position[Z_AXIS]);
		block -> steps_e = labs(target[E_AXIS] - position[E_AXIS]);
		block -> steps_e *= extrudemultiply;
		block -> steps_e /= 100;
		block -> step_event_count = max(block -> steps_x, max(block -> steps_y, max(block -> steps_z, block -> steps_e)));
		
//		x_step = x_step + block->steps_x;
//		y_step = y_step + block->steps_y;
//		usart_putstrln("block -> steps_x");
//		USART_Put_I2A(block -> steps_x);
//		usart_putstrln("block -> steps_y");
//		USART_Put_I2A(block -> steps_y);
//		usart_putstrln("block -> steps_z");
//		USART_Put_I2A(block -> steps_z);
//		GUI_DispDecAt(block -> steps_x, 10, 230, 3);
		if(block -> step_event_count <= dropsegments)
		{
//				GUI_DispStringAt("I will return!", 10, 210);
//				GUI_DispDecAt(block -> step_event_count, 10, 220, 3);
				return;
		}
//		x_step = x_step + block->steps_x;
//		y_step = y_step + block->steps_y;
		
		block -> fan_speed = fanSpeed;
		
		block -> direction_bits = 0;
		
		if(target[X_AXIS] < position[X_AXIS])
		{
				block -> direction_bits |= (1 << X_AXIS);
		}
		if(target[Y_AXIS] < position[Y_AXIS])
		{
				block -> direction_bits |= (1 << Y_AXIS);
		}
		if(target[Z_AXIS] < position[Z_AXIS])
		{
				block -> direction_bits |= (1 << Z_AXIS);
		}
		if(target[E_AXIS] < position[E_AXIS])
		{
				block -> direction_bits |= (1 << E_AXIS);
		}
//		usart_putstrln("outbits = ");
//		USART_Put_I2A(block->direction_bits);
		block -> active_extruder = extruder;
		
		if(block -> steps_x != 0)
				enable_x();
		if(block -> steps_y != 0)
				enable_y();
		if(block -> steps_z != 0)
				enable_z();
		if(block -> steps_e != 0)
		{
				enable_e0();
				enable_e1();
				enable_e2();
		}
		
		if(block -> steps_e == 0)
		{
				if(feed_rate < mintravelfeedrate)
						feed_rate = mintravelfeedrate;
		}
		else
		{
				if(feed_rate < minimumfeedrate)
						feed_rate = minimumfeedrate;
		}
		
		delta_mm[X_AXIS] = (target[X_AXIS]-position[X_AXIS])/axis_steps_per_unit[X_AXIS];
		delta_mm[Y_AXIS] = (target[Y_AXIS]-position[Y_AXIS])/axis_steps_per_unit[Y_AXIS];
		delta_mm[Z_AXIS] = (target[Z_AXIS]-position[Z_AXIS])/axis_steps_per_unit[Z_AXIS];
		delta_mm[E_AXIS] = ((target[E_AXIS]-position[E_AXIS])/axis_steps_per_unit[E_AXIS])*extrudemultiply/100.0;
		
		if ( block->steps_x <= dropsegments && block->steps_y <= dropsegments && block->steps_z <= dropsegments )
		{
				block->millimeters = fabs(delta_mm[E_AXIS]);
		} 
		else
		{
				block->millimeters = sqrt(square(delta_mm[X_AXIS]) + square(delta_mm[Y_AXIS]) + square(delta_mm[Z_AXIS]));
		}	
		
		inverse_millimeters = 1.0 / block -> millimeters;		// 1/mm
		inverse_second = feed_rate * inverse_millimeters;		// 1/s
		moves_queued=(block_buffer_head - block_buffer_tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);
		
		block -> nominal_speed = block -> millimeters * inverse_second;	// mm/s
		block -> nominal_rate = ceil(block->step_event_count * inverse_second);	// steps/s
		
		for(i = 0; i < 4; i++)
		{
				current_speed[i] = delta_mm[i] * inverse_second;	// mm/s
				if(fabs(current_speed[i]) > max_feedrate[i])
				speed_factor = min(speed_factor, max_feedrate[i] / fabs(current_speed[i]));
		}	
		
		if( speed_factor < 1.0)
		{
				for(i = 0; i < 4; i++)
				{
						current_speed[i] *= speed_factor;
				}
				block->nominal_speed *= speed_factor;
				block->nominal_rate *= speed_factor;
		}
		
		steps_per_mm = block->step_event_count/block->millimeters;	// steps/mm
		
		if(block->steps_x == 0 && block->steps_y == 0 && block->steps_z == 0)
		{
				block->acceleration_st = ceil(retract_acceleration * steps_per_mm);
		}
		
		else
		{		// mm/s^2 * steps/mm = steps/s^2
				block->acceleration_st = ceil(acceleration * steps_per_mm); // convert to: acceleration steps/sec^2
					// Limit acceleration per axis
				if(((float)block->acceleration_st * (float)block->steps_x / (float)block->step_event_count) > axis_steps_per_sqr_second[X_AXIS])
						block->acceleration_st = axis_steps_per_sqr_second[X_AXIS];
				if(((float)block->acceleration_st * (float)block->steps_y / (float)block->step_event_count) > axis_steps_per_sqr_second[Y_AXIS])
						block->acceleration_st = axis_steps_per_sqr_second[Y_AXIS];
				if(((float)block->acceleration_st * (float)block->steps_e / (float)block->step_event_count) > axis_steps_per_sqr_second[E_AXIS])
						block->acceleration_st = axis_steps_per_sqr_second[E_AXIS];
				if(((float)block->acceleration_st * (float)block->steps_z / (float)block->step_event_count ) > axis_steps_per_sqr_second[Z_AXIS])
						block->acceleration_st = axis_steps_per_sqr_second[Z_AXIS];
		}
			
		block->acceleration = block->acceleration_st / steps_per_mm;	// steps/s^2 / steps/mm = mm/s^2
		block->acceleration_rate = (long)((float)block->acceleration_st * 8.388608);
			
		vmax_junction = max_xy_jerk / 2; 
		vmax_junction_factor = 1.0;
			
		if(fabs(current_speed[Z_AXIS]) > max_z_jerk / 2) 
				vmax_junction = min(vmax_junction, max_z_jerk / 2);
		if(fabs(current_speed[E_AXIS]) > max_e_jerk / 2) 
				vmax_junction = min(vmax_junction, max_e_jerk / 2);
		vmax_junction = min(vmax_junction, block->nominal_speed);
		safe_speed = vmax_junction;
		
		if ((moves_queued > 1) && (previous_nominal_speed > 0.0001))
		{
				jerk = sqrt(pow((current_speed[X_AXIS] - previous_speed[X_AXIS]), 2)+pow((current_speed[Y_AXIS]-previous_speed[Y_AXIS]), 2));
				vmax_junction = block->nominal_speed;
				
				if (jerk > max_xy_jerk) 
				{
						vmax_junction_factor = (max_xy_jerk/jerk);
				} 
				if(fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS]) > max_z_jerk) 
				{
						vmax_junction_factor= min(vmax_junction_factor, (max_z_jerk/fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS])));
				}	 
				if(fabs(current_speed[E_AXIS] - previous_speed[E_AXIS]) > max_e_jerk) 
				{
						vmax_junction_factor = min(vmax_junction_factor, (max_e_jerk/fabs(current_speed[E_AXIS] - previous_speed[E_AXIS])));
				} 
				vmax_junction = min(previous_nominal_speed, vmax_junction * vmax_junction_factor); 
		}
		
		block -> max_entry_speed = vmax_junction;
		
		v_allowable = max_allowable_speed(-block->acceleration,MINIMUM_PLANNER_SPEED,block->millimeters);
		block->entry_speed = min(vmax_junction, v_allowable);
		
		if (block->nominal_speed <= v_allowable) 
		{ 
				block->nominal_length_flag = true; 
		}
		else 
		{ 
				block->nominal_length_flag = false; 
		}
		
		block->recalculate_flag = true;
		
		memcpy(previous_speed, current_speed, sizeof(previous_speed));
		previous_nominal_speed = block->nominal_speed;
		
		calculate_trapezoid_for_block(block, block->entry_speed/block->nominal_speed,
																	safe_speed/block->nominal_speed);
		block_buffer_head = next_buffer_head;
		memcpy(position, target, sizeof(target));
		planner_recalculate();
		st_wake_up();
}
