#ifdef _WIN32
#include <windows.h>
#include <winbase.h>

#include "dbugfile.h"
#include "timerbar.h"

typedef	struct 
{
	LARGE_INTEGER start_value;
	LARGE_INTEGER total_value;

	LARGE_INTEGER average;
	LARGE_INTEGER frame_total;

} timerbar_profile;

timerbar_profile profiles[TIMERBAR_COLOUR_MAX];
int timerbar_current_profile = 0;

LARGE_INTEGER timerbar_ultimate_start_value;
LARGE_INTEGER timerbar_last_start_value;

void (*draw_func_ptr)(int colour, float x, float y, float w, float h) = NULL;

void timerbar_start_frame()
{
	timerbar_current_profile = 0;

	if(QueryPerformanceCounter(&timerbar_ultimate_start_value) == FALSE)
	{
		DBUGFILE_OUTPUT_0("QueryPerformanceCounter not supported by hardware");
		draw_func_ptr = NULL;
	}

	timerbar_last_start_value = timerbar_ultimate_start_value;

}

const int WIDTH = 480;
const float WIDTHF = ((float) WIDTH) * 1.05f;

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


	for(int i = 0; i < TIMERBAR_COLOUR_MAX; i++)
	{		
		// Nothing to render this frame	 
		if(profiles[i].frame_total.QuadPart == 0)
		{
			continue;
		}

		int xsize = (int) profiles[i].frame_total.QuadPart / 60;

		// break condition in else
		for (;;)
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
	if(num >= TIMERBAR_COLOUR_MAX)
	{
		return;
	}


	// Calculate old profile's total
	LARGE_INTEGER now;
	if(QueryPerformanceCounter(&now) == FALSE)
	{
		DBUGFILE_OUTPUT_0("QueryPerformanceCounter not supported by hardware");
	}

	profiles[timerbar_current_profile].frame_total.QuadPart = 
		(now.QuadPart - timerbar_last_start_value.QuadPart);  

	if(num != -1)
	{
		// Switch to new profile
		timerbar_current_profile = num; 
	}

	// Update time to count from
	if(QueryPerformanceCounter(&timerbar_last_start_value) == FALSE)
	{
		DBUGFILE_OUTPUT_0("QueryPerformanceCounter not supported by hardware");
	}
}

void timerbar_set_draw_func(void (*new_draw_func_ptr)(int colour, float x, float y, float w, float h))
{
  	draw_func_ptr = new_draw_func_ptr;
}


#endif
