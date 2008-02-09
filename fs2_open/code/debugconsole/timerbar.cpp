/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

/*
 * This module is intended as a debug tool to measure the speed of blocks of code and display
 * that information as coloured bars across the top of the screen.
 *
 * This allows interactive measuring of components of the code, very useful for optimisztion.
 */

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#endif

#include "timerbar.h"
#include "globalincs/pstypes.h"


// Internal structure for handling frame data
typedef	struct 
{
	LARGE_INTEGER start_value;
	LARGE_INTEGER total_value;

	LARGE_INTEGER average;
	LARGE_INTEGER frame_total;

} timerbar_profile;

timerbar_profile profiles[MAX_NUM_TIMERBARS];
int timerbar_current_profile = 0;

LARGE_INTEGER timerbar_ultimate_start_value;
LARGE_INTEGER timerbar_last_start_value;

// Data needed for push and pop functionality
const int MAX_TB_STACK_SIZE = 100;

int timerbar_stack[MAX_TB_STACK_SIZE];
int timerbar_current_stack_layer = 0;

/*
 *
 *
 *
 */
void timerbar_push(int value)
{
	if(timerbar_current_stack_layer >= MAX_TB_STACK_SIZE)
	{
		// To many pushs, this must be a bug
		// Someone is letting code miss the pops, check for early retuns
		Assert(!(timerbar_current_stack_layer >= MAX_TB_STACK_SIZE));
		return;
	}

	timerbar_stack[timerbar_current_stack_layer] = timerbar_current_profile;
	timerbar_current_stack_layer++;

	timerbar_switch_type(value);
}

/*
 *
 *
 *
 */
void timerbar_pop()
{
	timerbar_current_stack_layer--;
	if(timerbar_current_stack_layer < 0)
	{
		Assert(!(timerbar_current_stack_layer < 0));
		timerbar_current_stack_layer = 0;
		return;
	}

	timerbar_switch_type(timerbar_stack[timerbar_current_stack_layer]);
}

// This pointer holds the draw function to use or NULL
void (*draw_func_ptr)(int colour, float x, float y, float w, float h) = NULL;

void timerbar_start_frame()
{
	timerbar_current_stack_layer = 0;
	timerbar_current_profile     = 0;

	if(QueryPerformanceCounter(&timerbar_ultimate_start_value) == FALSE)
	{
//		DBUGFILE_OUTPUT_0("QueryPerformanceCounter not supported by hardware");
		draw_func_ptr = NULL;
	}

	timerbar_last_start_value = timerbar_ultimate_start_value;

}

// Constants need for timerbar_conv_and_draw calculations
const int WIDTH = 480;
const float WIDTHF = ((float) WIDTH) * 1.01f;

/**
 * Internal conversion function to convert draw data into bars for the screen
 *
 * @param int colour - colour index (>=0)
 * @param int xpos   - x position to start bar on
 * @param int xwidth - width of bar
 * @param int yrow	 - Index of row bar belongs to
 */
void timerbar_conv_and_draw(int colour, int xpos, int xwidth, int yrow)
{
	float fxpos  = (float) xpos;
	float fxwidth = (float) xwidth;
		   
	draw_func_ptr(colour, 
		fxpos / WIDTHF,
		yrow * 0.01f,
		fxwidth / WIDTHF,
		0.005f);
}

void timerbar_end_frame()
{
	// Now we want to draw the bars
	if(draw_func_ptr == NULL)
	{
		// we have not been given a draw function so we are going to have to quit
		return;
	}

	timerbar_switch_type(-1);

	int last_xpos = 0;
	int y_pos     = 0;


	for(int i = 0; i < MAX_NUM_TIMERBARS; i++)
	{		
		// Nothing to render this frame	 
		if(profiles[i].frame_total.QuadPart == 0)
		{
			continue;
		}

		int xsize = (int) profiles[i].frame_total.QuadPart / 90;

		// break condition in else
		while(1)
		{
			// if bar runs over
			if(last_xpos + xsize > WIDTH)
			{
				int rem_len = WIDTH - last_xpos;

				// Draw from current position to the end of the bar
				timerbar_conv_and_draw(i, last_xpos, rem_len, y_pos);

				xsize -= rem_len;
				last_xpos = 0;
				y_pos++;
			}
			else
			{
				// Draw whats left and break for the next bar
				timerbar_conv_and_draw(i,last_xpos, xsize, y_pos);

				last_xpos += xsize;
				break;
			}
		}

		// reset for next frame
		profiles[i].frame_total.QuadPart = 0;
	}
}

void timerbar_switch_type(int num)
{
	if(num >= MAX_NUM_TIMERBARS)
	{
		return;
	}

	// Calculate old profile's total
	LARGE_INTEGER now;
	if(QueryPerformanceCounter(&now) == FALSE)
	{
//		DBUGFILE_OUTPUT_0("QueryPerformanceCounter not supported by hardware");
	}

	profiles[timerbar_current_profile].frame_total.QuadPart += 
		(now.QuadPart - timerbar_last_start_value.QuadPart);  

	if(num != -1)
	{
		// Switch to new profile
		timerbar_current_profile = num; 
	}

	// Update time to count from
	if(QueryPerformanceCounter(&timerbar_last_start_value) == FALSE)
	{
//		DBUGFILE_OUTPUT_0("QueryPerformanceCounter not supported by hardware");
	}
}

/**
 * @param void (*new_draw_func_ptr)(int colour, float x, float y, float w, float h) - pointer to draw function
 *
 * Sets the draw function used to output the timer data, designed to be generic allowing any API
 * to take advantage of this module. Set to NULL to disable drawing.
 *
 * By default the draw function is set to NULL
 */
void timerbar_set_draw_func(void (*new_draw_func_ptr)(int colour, float x, float y, float w, float h))
{
  	draw_func_ptr = new_draw_func_ptr;
}

