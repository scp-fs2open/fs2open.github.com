/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/2d.h $
 * $Revision: 2.11 $
 * $Date: 2003-10-24 17:35:05 $
 * $Author: randomtiger $
 *
 * Header file for 2d primitives.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.10  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.9  2003/10/21 18:23:15  phreak
 * added gr_flip_window back in.  its used in FRED
 *
 * Revision 2.8  2003/10/18 02:46:45  phreak
 * changed gr_start_instance_matrix(void) to gr_start_instance_matrix((vector*, matrix*)
 *
 * Revision 2.7  2003/10/17 17:18:42  randomtiger
 * Big restructure for D3D and new modules grd3dlight and grd3dsetup
 *
 * Revision 2.6  2003/10/13 19:39:19  matt
 * prelim reworking of lighting code, dynamic lights work properly now
 * albeit at only 8 lights per object, although it looks just as good as
 * the old software version --Sticks
 *
 * Revision 2.5  2003/10/10 03:59:40  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.4  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.3  2003/07/04 02:27:48  phreak
 * added support for cloaking.
 * i will need to contact someone who knows d3d to get this to work
 *
 * Revision 2.2  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.5  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.1.2.4  2002/11/04 16:04:20  randomtiger
 *
 * Tided up some bumpman stuff and added a few function points to gr_screen. - RT
 *
 * Revision 2.1.2.3  2002/11/04 03:02:28  randomtiger
 *
 * I have made some fairly drastic changes to the bumpman system. Now functionality can be engine dependant.
 * This is so D3D8 can call its own loading code that will allow good efficient loading and use of textures that it desparately needs without
 * turning bumpman.cpp into a total hook infested nightmare. Note the new bumpman code is still relying on a few of the of the old functions and all of the old bumpman arrays.
 *
 * I have done this by adding to the gr_screen list of function pointers that are set up by the engines init functions.
 * I have named the define calls the same name as the original 'bm_' functions so that I havent had to change names all through the code.
 *
 * Rolled back to an old version of bumpman and made a few changes.
 * Added new files: grd3dbumpman.cpp and .h
 * Moved the bitmap init function to after the 3D engine is initialised
 * Added includes where needed
 * Disabled (for now) the D3D8 TGA loading - RT
 *
 * Revision 2.1.2.2  2002/10/16 00:41:38  randomtiger
 * Fixed small bug that was stopping unactive text from displaying greyed out
 * Also added ability to run FS2 DX8 in 640x480, however I needed to make a small change to 2d.cpp
 * which invloved calling the resolution processing code after initialising the device for D3D only.
 * This is because D3D8 for the moment has its own internal launcher.
 * Also I added a fair bit of documentation and tidied some stuff up. - RT
 *
 * Revision 2.1.2.1  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 16    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 15    8/16/99 9:45a Jefff
 * changes to cursor management to allow a 2nd temporary cursor
 * 
 * 14    7/15/99 3:07p Dave
 * 32 bit detection support. Mouse coord commandline.
 * 
 * 13    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 12    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 11    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 10    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 9     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 8     1/24/99 11:36p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 7     12/21/98 5:02p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 6     12/18/98 1:49a Dave
 * Fixed Fred initialization problem resulting from hi-res mode changes.
 * 
 * 5     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 4     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 3     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 75    5/20/98 9:45p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 74    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 73    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 72    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 71    3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 70    3/24/98 8:31a John
 * Added function to set gamma
 * 
 * 69    3/17/98 5:55p John
 * Added code to dump Glide frames.   Moved Allender's  "hack" code out of
 * Freespace.cpp into the proper place, graphics lib.
 * 
 * 68    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 67    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 66    2/07/98 7:50p John
 * Added code so that we can use the old blending type of alphacolors if
 * we want to.  Made the stars use them.
 * 
 * 65    1/08/98 1:54p John
 * Added code to fix palette problems when Alt+Tabbing
 * 
 * 64    12/30/97 6:46p John
 * Added first rev of palette fade in out functions
 * 
 * 63    12/03/97 10:47a John
 * added functions to save/restore entire screens.
 * 
 * 62    12/02/97 3:59p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 61    11/20/97 9:51a John
 * added code to force screen to 16-bit even if rendering 8.
 * 
 * 60    11/03/97 10:08p Hoffoss
 * Changed gr_get_string_size to utilize an optional length specifier, if
 * you want to use non-null terminated strings.
 * 
 * 59    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 58    10/03/97 10:02a John
 * added better comments for lines.
 * 
 * 57    10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 56    9/23/97 10:45a John
 * made so you can tell bitblt code to rle a bitmap by passing flag to
 * gr_set_bitmap
 * 
 * 55    9/07/97 10:01p Lawrance
 * add in support for animating mouse pointer
 * 
 * 54    8/04/97 4:47p John
 * added gr_aascaler.
 * 
 * 53    7/16/97 3:07p John
 * 
 * 52    7/10/97 2:06p John
 * added code to specify alphablending type for bitmaps.
 * 
 * 51    6/25/97 2:35p John
 * added some functions to use the windows font for Fred.
 * 
 * 50    6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 49    6/17/97 12:03p John
 * Moved color/alphacolor functions into their own module.  Made all color
 * functions be part of the low-level graphics drivers, not just the
 * grsoft.
 * 
 * 48    6/13/97 5:35p John
 * added some antialiased bitmaps and lines
 * 
 * 47    6/11/97 5:49p John
 * Changed palette code to only recalculate alphacolors when needed, not
 * when palette changes.
 * 
 * 46    6/11/97 4:11p John
 * addec function to get font height
 * 
 * 45    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 44    6/09/97 9:24a John
 * Changed the way fonts are set.
 * 
 * 43    6/06/97 4:41p John
 * Fixed alpha colors to be smoothly integrated into gr_set_color_fast
 * code.
 * 
 * 42    6/05/97 4:53p John
 * First rev of new antialiased font stuff.
 * 
 * 41    5/29/97 3:09p John
 * Took out debug menu.  
 * Made software scaler draw larger bitmaps.
 * Optimized Direct3D some.
 * 
 * 40    5/14/97 10:53a John
 * fixed some discrepencies between d3d and software palette setting.
 * 
 * 39    5/12/97 3:09p John
 * fixed a stupid macro bug.
 * 
 * 38    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 37    4/28/97 4:46p John
 * 
 * 36    4/23/97 5:26p John
 * First rev of new debug console stuff.
 * 
 * 35    3/12/97 2:51p John
 * Added some test code for tmapper.  
 * 
 * 34    3/12/97 9:25a John
 * fixed a bug with zbuffering.  Reenabled it by default.
 * 
 * 33    3/04/97 3:36p John
 * took out old debug "h' key.   Made zbuffer flag bit bit field so you
 * can turn on/off each value.   Fixed a bug with turret rotation where
 * different handedness turrets wouldn't work.   Fixed a bug with two
 * large ships within each other's radius not rendering correctly.
 * 
 * 32    1/27/97 9:08a John
 * Added code to turn zbuffering on/off in call to g3_start_frame
 * 
 * 31    1/09/97 11:35a John
 * Added some 2d functions to get/put screen images.
 * 
 * 30    1/07/97 2:01p John
 * Fairly fast zbuffering for object sorting.
 * 
 * 29    1/06/97 2:44p John
 * Added in slow (but correct) zbuffering
 * 
 * 28    12/11/96 12:41p John
 * Added new code to draw 3d laser using 2d ellipses.
 * 
 * 27    12/10/96 10:37a John
 * Restructured texture mapper to remove some overhead from each scanline
 * setup.  This gave about a 30% improvement drawing trans01.pof, which is
 * a really complex model.  In the process, I cleaned up the scanline
 * functions and separated them into different modules for each pixel
 * depth.   
 * 
 * 26    11/21/96 11:21a John
 * Made gr_get_string_size handle multi line text.
 * Took out gr_get_multiline_string_size
 * 
 * 25    11/20/96 10:01a Hoffoss
 * A few minor improvements.
 * 
 * 24    11/18/96 4:35p Allender
 * new 16bpp gradient functions
 * 
 * 23    11/18/96 12:36p John
 * Added code to dump screen to a PCX file.
 * 
 * 22    11/18/96 11:40a John
 * Added faster gr_set_color method.
 * 
 * 21    11/15/96 3:34p Allender
 * added bpp variable to the shader structure
 * 
 * 20    11/13/96 6:47p John
 * Added gr_flip function.
 * 
 * 19    11/13/96 10:10a John
 * Increases MAX_WIDTH & HEIGHT for Jasen's massive 1600x1200 display.
 * 
 * 18    10/30/96 10:36a Lawrance
 * added gr_diamond function
 * 
 * 17    10/26/96 2:56p John
 * Got gradient code working.
 * 
 * 16    10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

/* ========================= pixel plotters =========================
In the 2d/texture mapper, bitmaps to be drawn will be passed by number.
The 2d function will call a bmpman function to get the bitmap into whatever
format it needs.  Then it will render.   The only pixels that will ever 
get drawn go thru the 2d/texture mapper libraries only.   This will make
supporting accelerators and psx easier.   Colors will always be set with
the color set functions.

gr_surface_flip()	switch onscreen, offscreen

gr_set_clip(x,y,w,h)	// sets the clipping region
gr_reset_clip(x,y,w,h)	// sets the clipping region
gr_set_color --? 8bpp, 15bpp?
gr_set_font(int fontnum)
// see GR_ALPHABLEND defines for values for alphablend_mode
// see GR_BITBLT_MODE defines for bitblt_mode.
// Alpha = scaler for intensity
gr_set_bitmap( int bitmap_num, int alphblend_mode, int bitblt_mode, float alpha )	
gr_set_shader( int value )  0=normal -256=darken, 256=brighten
gr_set_palette( ubyte * palette ) 

gr_clear()	// clears entire clipping region
gr_bitmap(x,y)
gr_bitmap_ex(x,y,w,h,sx,sy)
gr_rect(x,y,w,h)
gr_shade(x,y,w,h)
gr_string(x,y,char * text)
gr_line(x1,y1,x2,y2)

 
*/

#include "globalincs/pstypes.h"
#include "graphics/tmapper.h"

//MAX_POLYGON_NORMS
#define MAX_POLYGON_TRI_POINTS 5500

// This is a structure used by the shader to keep track
// of the values you want to use in the shade primitive.
typedef struct shader {
	uint	screen_sig;					// current mode this is in
	float	r,g,b,c;						// factors and constant
	ubyte	lookup[256];
} shader;

#define AC_TYPE_NONE		0		// Not an alphacolor
#define AC_TYPE_HUD		1		// Doesn't change hue depending on background.  Used for HUD stuff.
#define AC_TYPE_BLEND	2		// Changes hue depending on background.  Used for stars, etc.

// NEVER REFERENCE THESE VALUES OUTSIDE OF THE GRAPHICS LIBRARY!!!
// If you need to get the rgb values of a "color" struct call
// gr_get_colors after calling gr_set_colors_fast.
typedef struct color {
	uint		screen_sig;
	ubyte		red;
	ubyte		green;
	ubyte		blue;
	ubyte		alpha;
	ubyte		ac_type;							// The type of alphacolor.  See AC_TYPE_??? defines
	int		is_alphacolor;
	ubyte		raw8;
	int		alphacolor;
	int		magic;		
} color;

#define TL_COMPATABLE D3D_enabled || OGL_inited

//this should be basicly just like it is in the VB
//a list of triangles and there assosiated normals

struct poly_list{
	int n_poly;
	vertex vert[MAX_POLYGON_TRI_POINTS][3];
	vector norm[MAX_POLYGON_TRI_POINTS][3];
};

//exactly the same as the light structure from light.cpp, 
//I did this only becase it wouldn't compile from the header for some reason
//and becase we may need to add some data to it
struct light_data {
	int		type;							// What type of light this is
	vector	vec;							// location in world space of a point light or the direction of a directional light or the first point on the tube for a tube light
	vector	vec2;							// second point on a tube light
	vector	local_vec;					// rotated light vector
	vector	local_vec2;					// rotated 2nd light vector for a tube light
	float		intensity;					// How bright the light is.
	float		rada, rada_squared;		// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float		radb, radb_squared;		// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float		r,g,b;						// The color components of the light
	float		spec_r,spec_g,spec_b;		// The specular color components of the light
	int		ignore_objnum;				// Don't light this object.  Used to optimize weapons casting light on parents.
	int		affected_objnum;			// for "unique lights". ie, lights which only affect one object (trust me, its useful)
};


#define GR_ALPHABLEND_NONE			0		// no blending
#define GR_ALPHABLEND_FILTER		1		// 50/50 mix of foreground, background, using intensity as alpha

#define GR_BITBLT_MODE_NORMAL		0		// Normal bitblting
#define GR_BITBLT_MODE_RLE			1		// RLE would be faster

// fog modes
#define GR_FOGMODE_NONE				0		// set this to turn off fog
#define GR_FOGMODE_FOG				1		// linear fog

typedef struct screen {
	uint	signature;			// changes when mode or palette or width or height changes
	int	max_w, max_h;		// Width and height
	int	res;					// GR_640 or GR_1024
	int	mode;					// What mode gr_init was called with.
	float	aspect;				// Aspect ratio
	int	rowsize;				// What you need to add to go to next row (includes bytes_per_pixel)
	int	bits_per_pixel;	// How many bits per pixel it is. (7,8,15,16,24,32)
	int	bytes_per_pixel;	// How many bytes per pixel (1,2,3,4)
	int	offset_x, offset_y;		// The offsets into the screen
	int	clip_width, clip_height;

	float fog_near, fog_far;

	// the clip_l,r,t,b are used internally.  left and top are
	// actually always 0, but it's nice to have the code work with
	// arbitrary clipping regions.
	int		clip_left, clip_right, clip_top, clip_bottom;	

	int		current_alphablend_mode;		// See GR_ALPHABLEND defines above
	int		current_bitblt_mode;				// See GR_BITBLT_MODE defines above
	int		current_fog_mode;					// See GR_FOGMODE_* defines above
	int		current_bitmap;
	int		current_bitmap_sx;				// bitmap x section
	int		current_bitmap_sy;				// bitmap y section
	color		current_color;
	color		current_fog_color;				// current fog color
	color		current_clear_color;				// current clear color
	shader	current_shader;
	float		current_alpha;
	void		*offscreen_buffer;				// NEVER ACCESS!  This+rowsize*y = screen offset
	void		*offscreen_buffer_base;			// Pointer to lowest address of offscreen buffer

	//switch onscreen, offscreen
	void (*gf_flip)();
	void (*gf_flip_window)(uint _hdc, int x, int y, int w, int h );

	// Sets the current palette
	void (*gf_set_palette)(ubyte * new_pal, int restrict_alphacolor = 0);

	// Fade the screen in/out
	void (*gf_fade_in)(int instantaneous);
	void (*gf_fade_out)(int instantaneous);

	// Flash the screen
	void (*gf_flash)( int r, int g, int b );

	// sets the clipping region
	void (*gf_set_clip)(int x, int y, int w, int h);

	// resets the clipping region to entire screen
	void (*gf_reset_clip)();

	void (*gf_set_color)( int r, int g, int b );
	void (*gf_get_color)( int * r, int * g, int * b );
	void (*gf_init_color)( color * dst, int r, int g, int b );

	void (*gf_init_alphacolor)( color * dst, int r, int g, int b, int alpha, int type=AC_TYPE_HUD );
	void (*gf_set_color_fast)( color * dst );

	void (*gf_set_font)(int fontnum);

	// Sets the current bitmap
	void (*gf_set_bitmap)( int bitmap_num, int alphablend=GR_ALPHABLEND_NONE, int bitbltmode=GR_BITBLT_MODE_NORMAL, float alpha=1.0f, int sx = -1, int sy = -1 );

	// Call this to create a shader.   
	// This function takes a while, so don't call it once a frame!
	// r,g,b, and c should be between -1.0 and 1.0f

	// The matrix is used as follows:
	// Dest(r) = Src(r)*r + Src(g)*r + Src(b)*r + c;
	// Dest(g) = Src(r)*g + Src(g)*g + Src(b)*g + c;
	// Dest(b) = Src(r)*b + Src(g)*b + Src(b)*b + c;
	// For instance, to convert to greyscale, use
	// .3 .3 .3  0
	// To turn everything green, use:
	//  0 .3  0  0
	void (*gf_create_shader)(shader * shade, float r, float g, float b, float c );

	// Initialize the "shader" by calling gr_create_shader()
	// Passing a NULL makes a shader that turns everything black.
	void (*gf_set_shader)( shader * shade );

	// clears entire clipping region to current color
	void (*gf_clear)();

	// void (*gf_bitmap)(int x,int y);
	// void (*gf_bitmap_ex)(int x,int y,int w,int h,int sx,int sy);

	void (*gf_aabitmap)(int x, int y);
	void (*gf_aabitmap_ex)(int x, int y, int w, int h, int sx, int sy);

	void (*gf_rect)(int x, int y, int w, int h);
	void (*gf_shade)(int x, int y, int w, int h);
	void (*gf_string)(int x, int y, char * text);

	// Draw a gradient line... x1,y1 is bright, x2,y2 is transparent.
	void (*gf_gradient)(int x1, int y1, int x2, int y2);
 
	void (*gf_circle)(int x, int y, int r);

	// Integer line. Used to draw a fast but pixely line.  
	void (*gf_line)(int x1, int y1, int x2, int y2);

	// Draws an antialiased line is the current color is an 
	// alphacolor, otherwise just draws a fast line.  This
	// gets called internally by g3_draw_line.   This assumes
	// the vertex's are already clipped, so call g3_draw_line
	// not this if you have two 3d points.
	void (*gf_aaline)(vertex *v1, vertex *v2);

	void (*gf_pixel)( int x, int y );

	// Scales current bitmap between va and vb with clipping
	void (*gf_scaler)(vertex *va, vertex *vb );

	// Scales current bitmap between va and vb with clipping, draws an aabitmap
	void (*gf_aascaler)(vertex *va, vertex *vb );

	// Texture maps the current bitmap.  See TMAP_FLAG_?? defines for flag values
	void (*gf_tmapper)(int nv, vertex *verts[], uint flags );

	// dumps the current screen to a file
	void (*gf_print_screen)(char * filename);

	// Call once before rendering anything.
	void (*gf_start_frame)();

	// Call after rendering is over.
	void (*gf_stop_frame)();

	// Retrieves the zbuffer mode.
	int (*gf_zbuffer_get)();

	// Sets mode.  Returns previous mode.
	int (*gf_zbuffer_set)(int mode);

	// Clears the zbuffer.  If use_zbuffer is FALSE, then zbuffering mode is ignored and zbuffer is always off.
	void (*gf_zbuffer_clear)(int use_zbuffer);
	
	// Saves screen. Returns an id you pass to restore and free.
	int (*gf_save_screen)();
	
	// Resets clip region and copies entire saved screen to the screen.
	void (*gf_restore_screen)(int id);

	// Frees up a saved screen.
	void (*gf_free_screen)(int id);

	// CODE FOR DUMPING FRAMES TO A FILE
	// Begin frame dumping
	void (*gf_dump_frame_start)( int first_frame_number, int nframes_between_dumps );

	// Dump the current frame to file
	void (*gf_dump_frame)();

	// Dump the current frame to file
	void (*gf_dump_frame_stop)();

	// Sets the gamma
	void (*gf_set_gamma)(float gamma);

	// Lock/unlock the screen
	// Returns non-zero if sucessful (memory pointer)
	uint (*gf_lock)();
	void (*gf_unlock)();

	// grab a region of the screen. assumes data is large enough
	void (*gf_get_region)(int front, int w, int h, ubyte *data);

	// set fog attributes
	void (*gf_fog_set)(int fog_mode, int r, int g, int b, float fog_near = -1.0f, float fog_far = -1.0f);	

	// get the current pixel color in the framebuffer 
	void (*gf_get_pixel)(int x, int y, int *r, int *g, int *b);

	// poly culling
	void (*gf_set_cull)(int cull);

	// cross fade
	void (*gf_cross_fade)(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);

	// filtering
	void (*gf_filter_set)(int filter);

	// set a texture into cache. for sectioned bitmaps, pass in sx and sy to set that particular section of the bitmap
	int (*gf_tcache_set)(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0);	

	// set the color to be used when clearing the background
	void (*gf_set_clear_color)(int r, int g, int b);

	// Here be the bitmap functions
	void (*gf_bm_set_max_bitmap_size)(int size);
	int (*gf_bm_get_next_handle)();
	void (*gf_bm_close)();
	void (*gf_bm_init)();
	void (*gf_bm_get_frame_usage)(int *ntotal, int *nnew);
	int (*gf_bm_create)( int bpp, int w, int h, void * data, int flags = 0);
	int (*gf_bm_load)( char * real_filename );
	int (*gf_bm_load_duplicate)(char *filename);
	int (*gf_bm_load_animation)( char *real_filename, int *nframes, int *fps = NULL, int can_drop_frames = 0);
	void (*gf_bm_get_info)( int bitmapnum, int *w=NULL, int * h=NULL, ubyte * flags=NULL, int *nframes=NULL, int *fps=NULL, bitmap_section_info **sections = NULL);
	bitmap * (*gf_bm_lock)( int handle, ubyte bpp, ubyte flags );
	void (*gf_bm_unlock)( int handle );
	void (*gf_bm_get_palette)(int handle, ubyte *pal, char *name);
	void (*gf_bm_release)(int handle);
	int (*gf_bm_unload)( int handle );
	void (*gf_bm_unload_all)();
	void (*gf_bm_page_in_texture)( int bitmapnum, int nframes = 1);
	void (*gf_bm_page_in_start)();
	void (*gf_bm_page_in_stop)();
	int (*gf_bm_get_cache_slot)( int bitmap_id, int separate_ani_frames );
	void (*gf_bm_get_components)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
	void (*gf_bm_get_section_size)(int bitmapnum, int sx, int sy, int *w, int *h);

	void (*gf_bm_page_in_nondarkening_texture)( int bitmapnum, int nframes = 1);
	void (*gf_bm_page_in_xparent_texture)( int bitmapnum, int nframes = 1);
	void (*gf_bm_page_in_aabitmap)( int bitmapnum, int nframes = 1);

	void (*gf_translate_texture_matrix)(int unit, vector *shift);
	void (*gf_push_texture_matrix)(int unit);
	void (*gf_pop_texture_matrix)(int unit);

	int	 (*gf_make_buffer)(poly_list*);
	void (*gf_destroy_buffer)(int);
	void (*gf_render_buffer)(int);

 	void (*gf_start_instance_matrix)();
	void (*gf_end_instance_matrix)();

	int	 (*gf_make_light)(light_data*, int, int );
	void (*gf_modify_light)(light_data*, int, int );
	void (*gf_destroy_light)(int);
	void (*gf_set_light)(light_data*);
	void (*gf_reset_lighting)();

	void (*gf_lighting)(bool);

	void (*start_clip_plane)();
	void (*end_clip_plane)();

} screen;

// cpu types
extern int Gr_amd3d;
extern int Gr_katmai;
extern int Gr_cpu;	
extern int Gr_mmx;

// handy macro
#define GR_MAYBE_CLEAR_RES(bmap)		do  { int bmw = -1; int bmh = -1; if(bmap != -1){ bm_get_info( bmap, &bmw, &bmh); if((bmw != gr_screen.max_w) || (bmh != gr_screen.max_h)){gr_clear();} } else {gr_clear();} } while(0);

//Window's interface to set up graphics:
//--------------------------------------
// Call this at application startup

#define GR_SOFTWARE					(100)		// Software renderer using standard Win32 functions in a window.
#define GR_DIRECTDRAW				(101)		// Software renderer using DirectDraw fullscreen.
#define GR_DIRECT3D					(102)		// Use Direct3d hardware renderer
#define GR_GLIDE						(103)		// Use Glide hardware renderer
#define GR_OPENGL						(104)		// Use OpenGl hardware renderer

// resolution constants   - always keep resolutions in ascending order and starting from 0  
#define GR_NUM_RESOLUTIONS			2
#define GR_640							0		// 640 x 480
#define GR_1024						1		// 1024 x 768

extern int gr_init(int res, int mode, int depth = 16, int fred_x = -1, int fred_y = -1 );

// Call this when your app ends.
extern void gr_close();

extern screen gr_screen;

#define GR_ZBUFF_NONE	0
#define GR_ZBUFF_WRITE	(1<<0)
#define GR_ZBUFF_READ	(1<<1)
#define GR_ZBUFF_FULL	(GR_ZBUFF_WRITE|GR_ZBUFF_READ)

// Returns -1 if couldn't init font, otherwise returns the
// font id number.  If you call this twice with the same typeface,
// it will return the same font number both times.  This font is
// then set to be the current font, and default font if none is 
// yet specified.
int gr_init_font( char * typeface );

// Does formatted printing.  This calls gr_string after formatting,
// so if you don't need to format the string, then call gr_string
// directly.
extern void _cdecl gr_printf( int x, int y, char * format, ... );

// Returns the size of the string in pixels in w and h
extern void gr_get_string_size( int *w, int *h, char * text, int len = 9999 );

// Returns the height of the current font
extern int gr_get_font_height();

extern void gr_set_palette(char *name, ubyte *palette, int restrict_to_128 = 0);

// These two functions use a Windows mono font.  Only for use
// in the editor, please.
void gr_get_string_size_win(int *w, int *h, char *text);
void gr_string_win(int x, int y, char *s );

// set the mouse pointer to a specific bitmap, used for animating cursors
#define GR_CURSOR_LOCK		1
#define GR_CURSOR_UNLOCK	2
void gr_set_cursor_bitmap(int n, int lock = 0);
int gr_get_cursor_bitmap();
extern int Web_cursor_bitmap;

// Called by OS when application gets/looses focus
extern void gr_activate(int active);

// Sets up resolution
void gr_init_res(int res, int mode, int fredx = -1, int fredy = -1);

#define GR_CALL(x)			(*x)

// These macros make the function indirection look like the
// old Descent-style gr_xxx calls.

#define gr_print_screen		GR_CALL(gr_screen.gf_print_screen)

#define gr_flip				GR_CALL(gr_screen.gf_flip)
#define gr_flip_window		GR_CALL(gr_screen.gf_flip_window)

#define gr_set_clip			GR_CALL(gr_screen.gf_set_clip)
#define gr_reset_clip		GR_CALL(gr_screen.gf_reset_clip)
#define gr_set_font			GR_CALL(gr_screen.gf_set_font)

#define gr_init_color		GR_CALL(gr_screen.gf_init_color)
#define gr_init_alphacolor	GR_CALL(gr_screen.gf_init_alphacolor)
#define gr_set_color			GR_CALL(gr_screen.gf_set_color)
#define gr_get_color			GR_CALL(gr_screen.gf_get_color)
#define gr_set_color_fast	GR_CALL(gr_screen.gf_set_color_fast)

#define gr_set_bitmap		GR_CALL(gr_screen.gf_set_bitmap)

#define gr_create_shader	GR_CALL(gr_screen.gf_create_shader)
#define gr_set_shader		GR_CALL(gr_screen.gf_set_shader)
#define gr_clear				GR_CALL(gr_screen.gf_clear)
#define gr_aabitmap			GR_CALL(gr_screen.gf_aabitmap)
#define gr_aabitmap_ex		GR_CALL(gr_screen.gf_aabitmap_ex)
#define gr_rect				GR_CALL(gr_screen.gf_rect)
#define gr_shade				GR_CALL(gr_screen.gf_shade)
#define gr_string				GR_CALL(gr_screen.gf_string)

#define gr_circle				GR_CALL(gr_screen.gf_circle)

#define gr_line				GR_CALL(gr_screen.gf_line)
#define gr_aaline				GR_CALL(gr_screen.gf_aaline)
#define gr_pixel				GR_CALL(gr_screen.gf_pixel)
#define gr_scaler				GR_CALL(gr_screen.gf_scaler)
#define gr_aascaler			GR_CALL(gr_screen.gf_aascaler)
#define gr_tmapper			GR_CALL(gr_screen.gf_tmapper)

#define gr_gradient			GR_CALL(gr_screen.gf_gradient)


#define gr_fade_in			GR_CALL(gr_screen.gf_fade_in)
#define gr_fade_out			GR_CALL(gr_screen.gf_fade_out)
#define gr_flash				GR_CALL(gr_screen.gf_flash)

#define gr_zbuffer_get		GR_CALL(gr_screen.gf_zbuffer_get)
#define gr_zbuffer_set		GR_CALL(gr_screen.gf_zbuffer_set)
#define gr_zbuffer_clear	GR_CALL(gr_screen.gf_zbuffer_clear)

#define gr_save_screen		GR_CALL(gr_screen.gf_save_screen)
#define gr_restore_screen	GR_CALL(gr_screen.gf_restore_screen)
#define gr_free_screen		GR_CALL(gr_screen.gf_free_screen)

#define gr_dump_frame_start	GR_CALL(gr_screen.gf_dump_frame_start)
#define gr_dump_frame_stop		GR_CALL(gr_screen.gf_dump_frame_stop)
#define gr_dump_frame			GR_CALL(gr_screen.gf_dump_frame)

#define gr_set_gamma			GR_CALL(gr_screen.gf_set_gamma)

#define gr_lock				GR_CALL(gr_screen.gf_lock)
#define gr_unlock				GR_CALL(gr_screen.gf_unlock)

#define gr_get_region		GR_CALL(gr_screen.gf_get_region)

#define gr_fog_set			GR_CALL(gr_screen.gf_fog_set)

#define gr_set_cull			GR_CALL(gr_screen.gf_set_cull)

#define gr_cross_fade		GR_CALL(gr_screen.gf_cross_fade)

#define gr_filter_set		GR_CALL(gr_screen.gf_filter_set)

#define gr_tcache_set		GR_CALL(gr_screen.gf_tcache_set)

#define gr_set_clear_color	GR_CALL(gr_screen.gf_set_clear_color)

#define gr_translate_texture_matrix		GR_CALL(gr_screen.gf_translate_texture_matrix)
#define gr_push_texture_matrix			GR_CALL(gr_screen.gf_push_texture_matrix)
#define gr_pop_texture_matrix			GR_CALL(gr_screen.gf_pop_texture_matrix)


// Here be the bitmap functions
#define bm_set_max_bitmap_size     GR_CALL(*gr_screen.gf_bm_set_max_bitmap_size)
#define bm_get_next_handle         GR_CALL(*gr_screen.gf_bm_get_next_handle)
#define bm_close                   GR_CALL(*gr_screen.gf_bm_close)
#define bm_init                    GR_CALL(*gr_screen.gf_bm_init)
#define bm_get_frame_usage         GR_CALL(*gr_screen.gf_bm_get_frame_usage)
#define bm_create                  GR_CALL(*gr_screen.gf_bm_create)
#define bm_load                    GR_CALL(*gr_screen.gf_bm_load)
#define bm_load_duplicate          GR_CALL(*gr_screen.gf_bm_load_duplicate)
#define bm_load_animation          GR_CALL(*gr_screen.gf_bm_load_animation)
#define bm_get_info                GR_CALL(*gr_screen.gf_bm_get_info)
#define bm_lock                    GR_CALL(*gr_screen.gf_bm_lock)
#define bm_unlock                  GR_CALL(*gr_screen.gf_bm_unlock)
#define bm_get_palette             GR_CALL(*gr_screen.gf_bm_get_palette)
#define bm_release                 GR_CALL(*gr_screen.gf_bm_release)
#define bm_unload                  GR_CALL(*gr_screen.gf_bm_unload)
#define bm_unload_all              GR_CALL(*gr_screen.gf_bm_unload_all)
#define bm_page_in_texture         GR_CALL(*gr_screen.gf_bm_page_in_texture)
#define bm_page_in_start           GR_CALL(*gr_screen.gf_bm_page_in_start)
#define bm_page_in_stop            GR_CALL(*gr_screen.gf_bm_page_in_stop)
#define bm_get_cache_slot          GR_CALL(*gr_screen.gf_bm_get_cache_slot)
#define bm_get_components          GR_CALL(*gr_screen.gf_bm_get_components)
#define bm_get_section_size        GR_CALL(*gr_screen.gf_bm_get_section_size)

#define bm_page_in_nondarkening_texture  GR_CALL(*gr_screen.gf_bm_page_in_nondarkening_texture)
#define bm_page_in_xparent_texture 		 GR_CALL(*gr_screen.gf_bm_page_in_xparent_texture)     
#define bm_page_in_aabitmap				 GR_CALL(*gr_screen.gf_bm_page_in_aabitmap)            

#define gr_make_buffer					 GR_CALL(*gr_screen.gf_make_buffer)            
#define gr_destroy_buffer				 GR_CALL(*gr_screen.gf_destroy_buffer)            
#define gr_render_buffer				 GR_CALL(*gr_screen.gf_render_buffer)            

#define gr_start_instance_matrix		 GR_CALL(*gr_screen.gf_start_instance_matrix)            
#define gr_end_instance_matrix		 GR_CALL(*gr_screen.gf_end_instance_matrix)            

#define	gr_make_light GR_CALL			(*gr_screen.gf_make_light)
#define	gr_modify_light GR_CALL			(*gr_screen.gf_modify_light)
#define	gr_destroy_light GR_CALL		(*gr_screen.gf_destroy_light)
#define	gr_set_light GR_CALL			(*gr_screen.gf_set_light)
#define gr_reset_lighting GR_CALL		(*gr_screen.gf_reset_lighting)

#define	gr_set_lighting GR_CALL			(*gr_screen.gf_lighting)

#define	gr_start_clip GR_CALL			(*gr_screen.start_clip_plane)
#define	gr_end_clip GR_CALL				(*gr_screen.end_clip_plane)


// new bitmap functions
extern int Gr_bitmap_poly;
void gr_bitmap(int x, int y);
void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy);

// special function for drawing polylines. this function is specifically intended for
// polylines where each section is no more than 90 degrees away from a previous section.
// Moreover, it is _really_ intended for use with 45 degree angles. 
void gr_pline_special(vector **pts, int num_pts, int thickness);

#endif

