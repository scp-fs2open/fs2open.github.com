

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGL.cpp $
 * $Revision: 2.53 $
 * $Date: 2004-01-17 21:59:53 $
 * $Author: randomtiger $
 *
 * Code that uses the OpenGL graphics library
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.52  2003/12/17 23:25:10  phreak
 * added a MAX_BUFFERS_PER_SUBMODEL define so it can be easily changed if we ever want to change the 16 texture limit
 *
 * Revision 2.51  2003/11/29 16:11:46  fryday
 * Fixed normal loading in OpenGL HT&L.
 * Fixed lighting in OpenGL HT&L, hopefully for the last time.
 * Added a test if a normal is valid during model load, if not, replaced with face normal
 *
 * Revision 2.50  2003/11/25 15:04:45  fryday
 * Got lasers to work in HT&L OpenGL
 * Messed a bit with opengl_tmapper_internal3d, draw_laser functions, and added draw_laser_htl
 *
 * Revision 2.49  2003/11/22 10:36:32  fryday
 * Changed default alpha value for lasers in OpenGL to be like in D3D
 * Fixed Glowmaps being rendered with GL_NEAREST instead of GL_LINEAR.
 * Dynamic Lighting almost there. It looks like some normals are fudged or something.
 *
 * Revision 2.48  2003/11/17 04:25:56  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.47  2003/11/12 00:44:52  Kazan
 * (Kazan) /me slaps forehead... make sure things compile before committing.. sorry guys
 *
 * Revision 2.46  2003/11/11 18:05:02  phreak
 * support for GL_ARB_vertex_buffer_object.  should run real fast now with _VERY_
 * high poly ships
 *
 * Revision 2.45  2003/11/07 20:55:15  phreak
 * reenabled timerbar
 *
 * Revision 2.44  2003/11/06 22:36:04  phreak
 * bob's stack functions implemented in opengl
 *
 * Revision 2.43  2003/11/01 21:59:21  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.42  2003/10/29 02:09:18  randomtiger
 * Updated timerbar code to work properly, also added support for it in OGL.
 * In D3D red is general processing (and generic graphics), green is 2D rendering, dark blue is 3D unlit, light blue is HT&L renders and yellow is the presentation of the frame to D3D. OGL is all red for now. Use compile flag TIMERBAR_ON with code lib to activate it.
 * Also updated some more D3D device stuff that might get a bit more speed out of some cards.
 *
 * Revision 2.41  2003/10/25 03:26:39  phreak
 * fixed some old bugs that reappeared after RT committed his texture code
 *
 * Revision 2.40  2003/10/24 17:35:05  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.39  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.38  2003/10/20 22:32:37  phreak
 * cleaned up a bunch of repeated code
 *
 * Revision 2.37  2003/10/19 01:10:05  phreak
 * clipping planes now work
 *
 * Revision 2.36  2003/10/18 01:22:39  phreak
 * hardware transformation is working.  now lighting needs to be done
 *
 * Revision 2.35  2003/10/14 17:39:13  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.34  2003/10/13 19:39:19  matt
 * prelim reworking of lighting code, dynamic lights work properly now
 * albeit at only 8 lights per object, although it looks just as good as
 * the old software version --Sticks
 *
 * Revision 2.33  2003/10/13 05:57:48  Kazan
 * Removed a bunch of Useless *_printf()s in the rendering pipeline that were just slowing stuff down
 * Commented out the "warning null vector in vector normalize" crap since we don't give a rats arse
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 * Revision 2.32  2003/10/10 03:59:41  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.31  2003/10/05 23:40:54  phreak
 * bug squashing.
 * preliminary tnl work done
 *
 * Revision 2.30  2003/10/04 00:26:43  phreak
 * shinemapping completed. it works!
 * -phreak
 *
 * Revision 2.29  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.28  2003/08/21 15:07:45  phreak
 * added specular highlights. still needs shine mapping
 *
 * Revision 2.27  2003/08/06 17:26:20  phreak
 * changed default texture filtering to GL_LINEAR
 *
 * Revision 2.26  2003/08/03 23:35:36  phreak
 * changed some z-buffer stuff so it doesn't clip as noticably
 *
 * Revision 2.25  2003/07/15 02:34:59  phreak
 * fun work optimizing the cloak effect
 *
 * Revision 2.24  2003/07/04 02:27:48  phreak
 * added support for cloaking.
 * i will need to contact someone who knows d3d to get this to work
 *
 * Revision 2.23  2003/06/08 17:29:51  phreak
 * fixed a compile error that was accidently committed
 *
 * Revision 2.22  2003/06/07 20:51:06  phreak
 * fixed minor fogging bug
 *
 * Revision 2.21  2003/05/21 20:23:00  phreak
 * improved glowmap rendering speed
 *
 * Revision 2.20  2003/05/07 00:52:36  phreak
 * fixed old bug that created "mouse trails" during a popup screen
 * a result of doing something a long time ago that i thought was right, but wasn't
 *
 * Revision 2.19  2003/05/04 23:52:40  phreak
 * added additional error info incase gr_opengl_flip() fails for some unknown reason
 *
 * Revision 2.18  2003/05/04 20:49:18  phreak
 * minor fix in gr_opengl_flip()
 *
 * should be end of the "unable to swap buffer" error
 *
 * Revision 2.17  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.16  2003/03/07 00:15:45  phreak
 * added some hacks that shutdown and restore opengl because of cutscenes
 *
 * Revision 2.15  2003/02/16 18:43:13  phreak
 * tweaked some more stuff
 *
 * Revision 2.14  2003/02/01 02:57:42  phreak
 * started to finalize before 3.50 release.
 * added support for GL_EXT_texture_filter_anisotropic
 *
 * Revision 2.13  2003/01/26 23:30:53  DTP
 * temporary fix, added some // so that we can do debug builds. i expect Goober5000 will fix it soon.
 *
 * Revision 2.12  2003/01/19 22:45:34  Goober5000
 * cleaned up build output a bit
 * --Goober5000
 *
 * Revision 2.11  2003/01/19 01:07:41  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.10  2003/01/18 19:49:45  phreak
 * texture mapper now supports DXTC compressed textures
 *
 * Revision 2.9  2003/01/09 21:19:54  phreak
 * glowmaps in OpenGL, yay!
 *
 * Revision 2.8  2002/12/23 19:25:39  phreak
 * dumb typo
 *
 * Revision 2.7  2002/12/23 17:52:51  phreak
 * added code that displays an error if user's OGL version is less than 1.2
 *
 * Revision 2.6  2002/12/16 23:28:52  phreak
 * optimized fullscreen/minimized functions.. alot faster
 *
 * Revision 2.5  2002/12/15 18:59:57  DTP
 * fixed a minor glitch, replaced <> with ", so that it takes project glXXX.h, and not compilers glXXX.h
 *
 * Revision 2.4  2002/12/14 16:14:52  phreak
 * copied from grgpenglw32x.cpp.
 *
 * Revision 1.7  2002/12/05 00:49:25  phreak
 * extension framework implemented(no extensions work YET) -phreak
 *
 * Revision 1.6  2002/11/22 20:54:16  phreak
 * finished off all remaining problems with 32 bit mode, fullscreen mode
 * and colors.  Still needs some tweaking, but works near perfect
 *
 * Revision 1.5  2002/11/18 21:31:36  phreak
 * complete overhaul, works in 16 bit mode -phreak
 *
 * Revision 1.4  2002/10/14 19:49:08  phreak
 * screenshots, yay!
 *
 * Revision 1.3  2002/10/13 21:43:24  phreak
 * further optimizations
 *
 * Revision 1.2  2002/10/12 17:48:11  phreak
 * fixed text
 *
 * Revision 1.1  2002/10/11 21:31:17  phreak
 * first run at opengl for w32, only useful in main hall, barracks and campaign room
 *
 * Revision 1.1.1.1  2002/06/06 06:25:26  drevil
 * initial import of June 6th, 2002 sources
 *
 * Revision 1.46  2002/06/05 04:03:32  relnev
 * finished cfilesystem.
 *
 * removed some old code.
 *
 * fixed mouse save off-by-one.
 *
 * sound cleanups.
 *
 * Revision 1.45  2002/06/03 09:25:37  relnev
 * implement mouse cursor and screen save/restore
 *
 * Revision 1.44  2002/06/02 18:46:59  relnev
 * updated
 *
 * Revision 1.43  2002/06/02 11:34:00  relnev
 * adjust z coords
 *
 * Revision 1.42  2002/06/02 10:28:17  relnev
 * fix texture handle leak
 * Revision 2.3.2.2  2002/11/04 16:04:21  randomtiger
 *
 * Tided up some bumpman stuff and added a few function points to gr_screen. - RT
 *
 * Revision 2.3.2.1  2002/11/04 03:02:29  randomtiger
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
 * Revision 2.3  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 1.41  2002/06/01 09:00:34  relnev
 * silly debug memmanager
 *
 * Revision 1.40  2002/06/01 07:12:33  relnev
 * a few NDEBUG updates.
 *
 * removed a few warnings.
 *
 * Revision 1.39  2002/06/01 05:33:15  relnev
 * copied more code over.
 *
 * added scissor clipping.
 *
 * Revision 1.38  2002/06/01 03:35:27  relnev
 * fix typo
 *
 * Revision 1.37  2002/06/01 03:32:00  relnev
 * fix texture loading mistake.
 *
 * enable some d3d stuff for opengl also
 *
 * Revision 1.36  2002/05/31 23:25:03  relnev
 * line fixes
 *
 * Revision 1.34  2002/05/31 22:15:22  relnev
 * BGRA
 *
 * Revision 1.33  2002/05/31 22:04:55  relnev
 * use d3d rect_internal
 *
 * Revision 1.32  2002/05/31 06:28:23  relnev
 * more stuff
 *
 * Revision 1.31  2002/05/31 06:04:39  relnev
 * fog
 *
 * Revision 1.30  2002/05/31 03:56:11  theoddone33
 * Change tmapper polygon winding and enable culling
 *
 * Revision 1.29  2002/05/31 03:34:02  theoddone33
 * Fix Keyboard
 * Add titlebar
 *
 * Revision 1.28  2002/05/31 00:06:59  relnev
 * minor change
 *
 * Revision 1.27  2002/05/30 23:46:29  theoddone33
 * some minor key changes (not necessarily fixes)
 *
 * Revision 1.26  2002/05/30 23:33:12  relnev
 * implemented a few more functions.
 *
 * Revision 1.25  2002/05/30 23:01:16  relnev
 * implement gr_opengl_set_state.
 *
 * Revision 1.24  2002/05/30 22:12:57  relnev
 * finish default texture case
 *
 * Revision 1.23  2002/05/30 22:02:30  theoddone33
 * More gl changes
 *
 * Revision 1.22  2002/05/30 21:44:48  relnev
 * implemented some missing texture stuff.
 *
 * enable bitmap polys for opengl.
 *
 * work around greenness in bitmaps.
 *
 * Revision 1.21  2002/05/30 17:29:30  theoddone33
 * Fix some more stubs, change at least one polygon winding since culling is now
 * enabled.
 *
 * Revision 1.20  2002/05/30 16:50:24  theoddone33
 * Keyboard partially fixed
 *
 * Revision 1.19  2002/05/30 08:13:14  relnev
 * fonts are fixed
 *
 * Revision 1.18  2002/05/29 23:37:36  relnev
 * fix bitmap bug
 *
 * Revision 1.17  2002/05/29 23:17:49  theoddone33
 * Non working text code and fixed keys
 *
 * Revision 1.16  2002/05/29 19:45:13  theoddone33
 * More changes on texture loading
 *
 * Revision 1.15  2002/05/29 19:06:48  theoddone33
 * Enable string printing.  Enable texture mapping
 *
 * Revision 1.14  2002/05/29 08:54:40  relnev
 * "fixed" bitmap drawing.
 *
 * copied more d3d code over.
 *
 * Revision 1.13  2002/05/29 06:25:13  theoddone33
 * Keyboard input, mouse tracking now work
 *
 * Revision 1.12  2002/05/29 04:52:45  relnev
 * bitmap
 *
 * Revision 1.11  2002/05/29 04:29:56  relnev
 * removed some unncessary stubbing, implemented opengl rect
 *
 * Revision 1.10  2002/05/29 04:13:27  theoddone33
 * enable opengl_line
 *
 * Revision 1.9  2002/05/29 03:35:51  relnev
 * added rest of init
 *
 * Revision 1.8  2002/05/29 03:30:05  relnev
 * update opengl stubs
 *
 * Revision 1.7  2002/05/29 02:52:32  theoddone33
 * Enable OpenGL renderer
 *
 * Revision 1.6  2002/05/28 04:56:51  theoddone33
 * runs a little bit now
 *
 * Revision 1.5  2002/05/28 04:07:28  theoddone33
 * New graphics stubbing arrangement
 *
 * Revision 1.4  2002/05/27 23:39:34  relnev
 * 0
 *
 * Revision 1.3  2002/05/27 22:35:01  theoddone33
 * more symbols
 *
 * Revision 1.2  2002/05/27 22:32:02  theoddone33
 * throw all d3d stuff at opengl
 *
 * Revision 1.1.1.1  2002/05/03 03:28:09  root
 * Initial import.
 *
 * 
 * 10    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 9     7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 8     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 7     2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 6     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
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
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 14    5/20/98 9:46p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 13    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 12    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 11    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 10    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 9     12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 8     10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 7     9/23/97 10:45a John
 * made so you can tell bitblt code to rle a bitmap by passing flag to
 * gr_set_bitmap
 * 
 * 6     9/09/97 11:01a Sandeep
 * fixed warning level 4 bugs
 * 
 * 5     7/10/97 2:06p John
 * added code to specify alphablending type for bitmaps.
 * 
 * 4     6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 3     6/12/97 2:50a Lawrance
 * bm_unlock() now passed bitmap number, not pointer
 * 
 * 2     6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 1     5/12/97 12:14p John
 *
 * $NoKeywords: $
 */

/*
This file combines penguin's, phreak's and the Icculus OpenGL code

		--STUFF WE STILL NEED TO DO--
		-----------------------------
1. Switch to true 3d graphics (not ortho)
2. Start extenstion implentation (last)

*/

#ifdef WIN32

#define STUB_FUNCTION 0

#include <windows.h>
#include <windowsx.h>

#include "gl/gl.h"
#include "gl/glu.h"
#include "gl/glext.h"

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/line.h"
#include "nebula/neb.h"
#include "io/mouse.h"
#include "osapi/osregistry.h"
#include "cfile/cfile.h"
#include "io/timer.h"
#include "ddsutils/ddsutils.h"
#include "model/model.h"
#include "debugconsole/timerbar.h"

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")


#define REQUIRED_GL_VERSION 1.2f

extern int OGL_inited;
extern int Cmdline_nohtl;

int vram_full = 0;			// UnknownPlayer

//0==no fog
//1==linear
//2==fog coord EXT
//3==NV Radial
static int OGL_fogmode=0;

static HDC dev_context = NULL;
static HGLRC rend_context = NULL;
static PIXELFORMATDESCRIPTOR pfd;

//EXTENSIONS!!!!
//be sure to check for this at startup and handle not finding it gracefully

//to add extensions:
//define an index after the last one
//increment GL_NUM_EXTENSIONS
//add function macro
//add function info to GL_Extensions struct
//I included three as examples

typedef struct ogl_extension
{
	int enabled;					//is this extension enabled
	uint func_pointer;				//address of function
	const char* function_name;		//name passed to wglGetProcAddress()
	const char* extension_name;		//name found in extension string
	int required_to_run;			//is this extension required for use	
} ogl_extension;

#define GL_FOG_COORDF					0			// for better looking fog
#define GL_FOG_COORD_POINTER			1			// used with vertex arrays
#define GL_MULTITEXTURE_COORD2F			2			// multitex coordinates
#define GL_ACTIVE_TEX					3			// currenly active multitexture
#define GL_TEXTURE_ENV_ADD				4			// additive texture environment
#define GL_COMP_TEX						5			// texture compression
#define GL_TEX_COMP_S3TC				6			// S3TC/DXTC compression format
#define GL_TEX_FILTER_ANSIO				7			// anisotrophic filtering
#define GL_NV_RADIAL_FOG				8			// for better looking fog
#define GL_SECONDARY_COLOR_3FV			9			// for better looking fog
#define GL_SECONDARY_COLOR_3UBV			10			// specular
#define GL_ARB_ENV_COMBINE				11			// spec mapping
#define GL_EXT_ENV_COMBINE				12			// spec mapping
#define GL_LOCK_ARRAYS					13			// HTL
#define GL_UNLOCK_ARRAYS				14			// HTL
#define GL_LOAD_TRANSPOSE				15			
#define GL_MULT_TRANSPOSE				16
#define GL_CLIENT_ACTIVE_TEX			17


//GL_ARB_vertex_buffer_object FUNCTIONS
#define GL_ARB_VBO_BIND_BUFFER			18			
#define GL_ARB_VBO_DEL_BUFFER			19
#define GL_ARB_VBO_GEN_BUFFER			20
#define GL_ARB_VBO_BUFFER_DATA			21
//#define GL_ARB_VBO_MAP_BUFFER			22
//#define GL_ARB_VBO_UNMAP_BUFFER			23


#define GL_NUM_EXTENSIONS				22

/*
Assorted Functions for GL_ARB_vertex_buffer_object

void BindBufferARB(enum target, uint buffer);
void DeleteBuffersARB(sizei n, const uint *buffers);
void GenBuffersARB(sizei n, uint *buffers);
boolean IsBufferARB(uint buffer);

void BufferDataARB(enum target, sizeiptrARB size, const void *data,
                       enum usage);
void BufferSubDataARB(enum target, intptrARB offset, sizeiptrARB size,
                          const void *data);
void GetBufferSubDataARB(enum target, intptrARB offset,
                             sizeiptrARB size, void *data);

void *MapBufferARB(enum target, enum access);
boolean UnmapBufferARB(enum target);

void GetBufferParameterivARB(enum target, enum pname, int *params);
void GetBufferPointervARB(enum target, enum pname, void **params);

  */

static ogl_extension GL_Extensions[GL_NUM_EXTENSIONS]=
{
	{0, NULL, "glFogCoordfEXT", "GL_EXT_fog_coord",0},
	{0, NULL, "glFogCoordPointerEXT", "GL_EXT_fog_coord",0},
	{0, NULL, "glMultiTexCoord2fARB", "GL_ARB_multitexture",1},		//required for glow maps
	{0, NULL, "glActiveTextureARB", "GL_ARB_multitexture",1},		//required for glow maps
	{0, NULL, NULL, "GL_ARB_texture_env_add", 1},					//required for glow maps
	{0, NULL, "glCompressedTexImage2D", "GL_ARB_texture_compression",0},
	{0, NULL, NULL, "GL_EXT_texture_compression_s3tc",0},
	{0, NULL, NULL, "GL_EXT_texture_filter_anisotropic", 0},
	{0, NULL, NULL, "GL_NV_fog_distance", 0},
	{0, NULL, "glSecondaryColor3fvEXT", "GL_EXT_secondary_color", 0},
	{0, NULL, "glSecondaryColor3ubvEXT", "GL_EXT_secondary_color", 0},
	{0, NULL, NULL, "GL_ARB_texture_env_combine",0},
	{0, NULL, NULL, "GL_EXT_texture_env_combine",0},
	{0, NULL, "glLockArraysEXT", "GL_EXT_compiled_vertex_array",0},
	{0, NULL, "glUnlockArraysEXT", "GL_EXT_compiled_vertex_array",0},
	{0, NULL,"glLoadTransposeMatrixfARB","GL_ARB_transpose_matrix",	1},
	{0, NULL, "glMultTransposeMatrixfARB", "GL_ARB_transpose_matrix",1},
	{0, NULL, "glClientActiveTextureARB", "GL_ARB_multitexture",1},
	{0, NULL, "glBindBufferARB", "GL_ARB_vertex_buffer_object",0},
	{0, NULL, "glDeleteBuffersARB", "GL_ARB_vertex_buffer_object",0},
	{0, NULL, "glGenBuffersARB", "GL_ARB_vertex_buffer_object",0},
	{0, NULL, "glBufferDataARB", "GL_ARB_vertex_buffer_object",0}
//	{0, NULL, "glMapBufferARB", "GL_ARB_vertex_buffer_object",0},
//	{0, NULL, "glUnmapBufferARB", "GL_ARB_vertex_buffer_object",0}
};

#define GLEXT_CALL(x,i) if (GL_Extensions[i].enabled)\
							((x)GL_Extensions[i].func_pointer)

#define glFogCoordfEXT GLEXT_CALL(PFNGLFOGCOORDFEXTPROC, GL_FOG_COORDF)

#define glFogCoordPointerEXT GLEXT_CALL(PFNGLFOGCOORDPOINTEREXTPROC, GL_FOG_COORD_POINTER);

#define glMultiTexCoord2fARB GLEXT_CALL(PFNGLMULTITEXCOORD2FARBPROC, GL_MULTITEXTURE_COORD2F)

#define glActiveTextureARB GLEXT_CALL(PFNGLACTIVETEXTUREARBPROC, GL_ACTIVE_TEX)

#define glCompressedTexImage2D GLEXT_CALL(PFNGLCOMPRESSEDTEXIMAGE2DPROC, GL_COMP_TEX)

#define glSecondaryColor3fvEXT GLEXT_CALL(PFNGLSECONDARYCOLOR3FVEXTPROC, GL_SECONDARY_COLOR_3FV)

#define glSecondaryColor3ubvEXT GLEXT_CALL(PFNGLSECONDARYCOLOR3UBVEXTPROC, GL_SECONDARY_COLOR_3UBV)

#define glLockArraysEXT GLEXT_CALL(PFNGLLOCKARRAYSEXTPROC, GL_LOCK_ARRAYS)

#define glUnlockArraysEXT GLEXT_CALL(PFNGLUNLOCKARRAYSEXTPROC, GL_UNLOCK_ARRAYS)
									 
#define glLoadTransposeMatrixfARB GLEXT_CALL(PFNGLLOADTRANSPOSEMATRIXFARBPROC, GL_LOAD_TRANSPOSE)

#define glMultTransposeMatrixfARB GLEXT_CALL(PFNGLMULTTRANSPOSEMATRIXFARBPROC, GL_MULT_TRANSPOSE)

#define glClientActiveTextureARB GLEXT_CALL(PFNGLCLIENTACTIVETEXTUREARBPROC, GL_CLIENT_ACTIVE_TEX)

#define glBindBufferARB GLEXT_CALL(PFNGLBINDBUFFERARBPROC, GL_ARB_VBO_BIND_BUFFER)
#define glDeleteBuffersARB GLEXT_CALL(PFNGLDELETEBUFFERSARBPROC, GL_ARB_VBO_DEL_BUFFER)
#define glGenBuffersARB GLEXT_CALL(PFNGLGENBUFFERSARBPROC, GL_ARB_VBO_GEN_BUFFER)
#define glBufferDataARB GLEXT_CALL(PFNGLBUFFERDATAARBPROC, GL_ARB_VBO_BUFFER_DATA)

static int VBO_ENABLED = 0;

extern int Texture_compression_enabled;

typedef enum gr_texture_source {
	TEXTURE_SOURCE_NONE=0,
	TEXTURE_SOURCE_DECAL,
	TEXTURE_SOURCE_NO_FILTERING,
	TEXTURE_SOURCE_MODULATE4X,
} gr_texture_source;

typedef enum gr_alpha_blend {
        ALPHA_BLEND_NONE=0,			// 1*SrcPixel + 0*DestPixel
        ALPHA_BLEND_ALPHA_ADDITIVE,             // Alpha*SrcPixel + 1*DestPixel
        ALPHA_BLEND_ALPHA_BLEND_ALPHA,          // Alpha*SrcPixel + (1-Alpha)*DestPixel
        ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR,      // Alpha*SrcPixel + (1-SrcPixel)*DestPixel
} gr_alpha_blend;

typedef enum gr_zbuffer_type {
        ZBUFFER_TYPE_NONE=0,
        ZBUFFER_TYPE_READ,
        ZBUFFER_TYPE_WRITE,
        ZBUFFER_TYPE_FULL,
} gr_zbuffer_type;
                        
#define NEBULA_COLORS 20

volatile int GL_activate = 0;
volatile int GL_deactivate = 0;

static char *Gr_saved_screen = NULL;
static int Gr_saved_screen_bitmap;

static int Gr_opengl_mouse_saved = 0;
static int Gr_opengl_mouse_saved_x1 = 0;
static int Gr_opengl_mouse_saved_y1 = 0;
static int Gr_opengl_mouse_saved_x2 = 0;
static int Gr_opengl_mouse_saved_y2 = 0;
static int Gr_opengl_mouse_saved_w = 0;
static int Gr_opengl_mouse_saved_h = 0;

extern int Cmdline_window;
extern int Interp_multitex_cloakmap;
extern int CLOAKMAP;
extern int Interp_cloakmap_alpha;

static const char* OGL_extensions;

static int arb0_enabled=1;
static int arb1_enabled=1;
static int arb2_enabled=1;

static int max_multitex;

static float max_aniso=1.0f;			//max anisotropic filtering ratio

static vector tex_shift;

void (*gr_opengl_tmapper_internal)( int nv, vertex ** verts, uint flags, int is_scaler ) = NULL;
void (*gr_opengl_set_tex_src)(gr_texture_source ts);

float *specular_color;

extern vector G3_user_clip_normal;
extern vector G3_user_clip_point;

int depth = 0;

//some globals
extern matrix View_matrix;
extern vector View_position;
extern matrix Eye_matrix;
extern vector Eye_position;
extern vector Object_position;
extern matrix Object_matrix;
extern float	Canv_w2;				// Canvas_width / 2
extern float	Canv_h2;				// Canvas_height / 2
extern float	View_zoom;
static int n_active_lights = 0;
const int MAX_OPENGL_LIGHTS = 8; //temporary - will change to dynamic allocation and get rid of this later -Fry_Day

enum
{
	LT_DIRECTIONAL,		// A light like a sun
	LT_POINT,			// A point light, like an explosion
	LT_TUBE,			// A tube light, like a fluorescent light
};

// Structures
struct opengl_light{
	opengl_light():occupied(false), priority(1){};
	struct {
		float r,g,b,a;
	} Diffuse, Specular, Ambient;
	struct {
		float x,y,z,w;
	} Position;
	float ConstantAtten, LinearAtten, QuadraticAtten;
	bool occupied;
	int priority;
};

// Variables

opengl_light opengl_lights[MAX_LIGHTS];
bool active_light_list[MAX_LIGHTS];
int currently_enabled_lights[MAX_OPENGL_LIGHTS] = {-1};

bool lighting_is_enabled = true;
extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;

int max_gl_lights;

void FSLight2GLLight(opengl_light *GLLight,light_data *FSLight) {

	GLLight->Diffuse.r = FSLight->r;// * FSLight->intensity;
	GLLight->Diffuse.g = FSLight->g;// * FSLight->intensity;
	GLLight->Diffuse.b = FSLight->b;// * FSLight->intensity;
	GLLight->Specular.r = FSLight->spec_r;// * FSLight->intensity;
	GLLight->Specular.g = FSLight->spec_g;// * FSLight->intensity;
	GLLight->Specular.b = FSLight->spec_b;// * FSLight->intensity;
	GLLight->Ambient.r = 0.0f;
	GLLight->Ambient.g = 0.0f;
	GLLight->Ambient.b = 0.0f;
	GLLight->Ambient.a = 1.0f;
	GLLight->Specular.a = 1.0f;
	GLLight->Diffuse.a = 1.0f;


	//If the light is a directional light
	if(FSLight->type == LT_DIRECTIONAL) {
		GLLight->Position.x = -FSLight->vec.xyz.x;
		GLLight->Position.y = -FSLight->vec.xyz.y;
		GLLight->Position.z = -FSLight->vec.xyz.z;
		GLLight->Position.w = 0.0f; //Directional lights in OpenGL have w set to 0 and the direction vector in the position field

		GLLight->Specular.r *= static_light_factor;
		GLLight->Specular.g *= static_light_factor;
		GLLight->Specular.b *= static_light_factor;
	}

	//If the light is a point or tube type
	if((FSLight->type == LT_POINT) || (FSLight->type == LT_TUBE)) {

		if(FSLight->type == LT_POINT){
			GLLight->Specular.r *= static_point_factor;
			GLLight->Specular.g *= static_point_factor;
			GLLight->Specular.b *= static_point_factor;
		}else{
			GLLight->Specular.r *= static_tube_factor;
			GLLight->Specular.g *= static_tube_factor;
			GLLight->Specular.b *= static_tube_factor;
		}

		GLLight->Position.x = FSLight->vec.xyz.x;
		GLLight->Position.y = FSLight->vec.xyz.y;
		GLLight->Position.z = FSLight->vec.xyz.z; //flipped axis for FS2
		GLLight->Position.w = 1.0f;		

		//They also have almost no radius...
//		GLLight->Range = FSLight->radb +FSLight->rada; //No range function in OpenGL that I'm aware of
		GLLight->ConstantAtten = 0.0f;
		GLLight->LinearAtten = 0.1f;
		GLLight->QuadraticAtten = 0.0f; 
	}

}

void set_opengl_light(int light_num, opengl_light *light)
{
	Assert(light_num < max_gl_lights);
	glLightfv(GL_LIGHT0+light_num, GL_POSITION, &light->Position.x);
	glLightfv(GL_LIGHT0+light_num, GL_AMBIENT, &light->Ambient.r);
	glLightfv(GL_LIGHT0+light_num, GL_DIFFUSE, &light->Diffuse.r);
	glLightfv(GL_LIGHT0+light_num, GL_SPECULAR, &light->Specular.r);
	glLightf(GL_LIGHT0+light_num, GL_CONSTANT_ATTENUATION, light->ConstantAtten);
	glLightf(GL_LIGHT0+light_num, GL_LINEAR_ATTENUATION, light->LinearAtten);
	glLightf(GL_LIGHT0+light_num, GL_QUADRATIC_ATTENUATION, light->QuadraticAtten);
}
//finds the first unocupyed light

void pre_render_init_lights(){
	for(int i = 0; i<max_gl_lights; i++){
		if(currently_enabled_lights[i] > -1) glDisable(GL_LIGHT0+i);
		currently_enabled_lights[i] = -1;
	}
}


void change_active_lights(int pos){
	int k = 0;
	int l = 0;
	if(!lighting_is_enabled)return;
	bool move = false;
	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix();				
	glLoadIdentity();

//straight cut'n'paste out of gr_opengl_set_view_matrix, but I couldn't use that, since it messes up with the stack depth var
	vector fwd;
	vector *uvec=&Eye_matrix.vec.uvec;

	vm_vec_add(&fwd, &Eye_position, &Eye_matrix.vec.fvec);

	gluLookAt(Eye_position.xyz.x,Eye_position.xyz.y,-Eye_position.xyz.z,
	fwd.xyz.x,fwd.xyz.y,-fwd.xyz.z,
	uvec->xyz.x, uvec->xyz.y,-uvec->xyz.z);

	glScalef(1,1,-1);
	
	for(int i = 0; (i < max_gl_lights) && ((pos * max_gl_lights)+i < n_active_lights); i++){
		glDisable(GL_LIGHT0+i);
		move = false;
		for(k; k<MAX_LIGHTS && !move; k++){
			int slot = (pos * max_gl_lights)+l;
			if(active_light_list[slot]){
				if(opengl_lights[slot].occupied){
					set_opengl_light(i,&opengl_lights[slot]);
					glEnable(GL_LIGHT0+i);
					currently_enabled_lights[i] = slot;
					move = true;
					l++;
				}
			}
		}
	}
	

	glPopMatrix();

}

int	gr_opengl_make_light(light_data* light, int idx, int priority)
{
//Stub
	return idx;
}

void gr_opengl_modify_light(light_data* light, int idx, int priority)
{
//Stub
}

void gr_opengl_destroy_light(int idx)
{
//Stub
}

void gr_opengl_set_light(light_data *light)
{
	//Init the light
	FSLight2GLLight(&opengl_lights[n_active_lights],light);
	opengl_lights[n_active_lights].occupied = true;
	active_light_list[n_active_lights++] = true;
}

void gr_opengl_reset_lighting()
{
	for(int i = 0; i<MAX_LIGHTS; i++){
		opengl_lights[i].occupied = false;
	}
	for(i=0; i<max_gl_lights; i++){
		glDisable(GL_LIGHT0+i);
		active_light_list[i] = false;
	}
	n_active_lights =0;
}


void gr_opengl_set_lighting(bool set, bool state)
{
	struct {
			float r,g,b,a;
	} ambient, col;

	lighting_is_enabled = set;

	col.r = col.g = col.b = col.a = set ? 1.0f : 0.0f;  //Adunno why the ambient and diffuse need to be set to 0.0 when lighting is disabled
														//They just do, and that should suffice as an answer
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE, &col.r); //changed to GL_FRONT_AND_BACK, just to make sure
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, &col.r); //changed to GL_FRONT_AND_BACK, just to make sure
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,(float)specular_exponent_value);
	if((gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) && !set){
		ambient.r = ambient.g = ambient.b = ambient.a = gr_screen.current_alpha;
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &ambient.r);
	}else{
		ambient.r = ambient.g = ambient.b = 0.125; // 1/16th of the max value, just like D3D
		ambient.a = 1.0f;
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &ambient.r);
	}

	for(int i = 0; i<max_gl_lights; i++){
		if(currently_enabled_lights[i] > -1)glDisable(GL_LIGHT0+i);
		currently_enabled_lights[i] = -1;
	}

	if(state) {
		glEnable(GL_LIGHTING);
	}
	else {
		glDisable(GL_LIGHTING);
	}

}
//End of lighting stuff
inline static void opengl_switch_arb0(int state)
{
	if (state)
	{
		if (arb0_enabled)	return;

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		arb0_enabled=1;
	}

	else
	{
		if (!arb0_enabled)	return;

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		arb0_enabled=0;
	}
}

inline static void opengl_switch_arb1(int state)
{
	if (state)
	{
		if (arb1_enabled)	return;

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		arb1_enabled=1;
	}

	else
	{
		if (!arb1_enabled)	return;

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		arb1_enabled=0;
	}
}

inline static void opengl_switch_arb2(int state)
{
	if (max_multitex == 2)
		return;

	if (state)
	{
		if (arb2_enabled)	return;

		glActiveTextureARB(GL_TEXTURE2_ARB);
		glEnable(GL_TEXTURE_2D);
		arb2_enabled=1;
	}

	else
	{
		if (!arb2_enabled)	return;

		glActiveTextureARB(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_2D);
		arb2_enabled=0;
	}
}

//tries to find a certain extension
inline static int opengl_find_extension(const char* ext_to_find)
{
	return (strstr(OGL_extensions,ext_to_find)!=NULL);
}


//finds OGL extension functions
//returns number found
static int opengl_get_extensions()
{
	int num_found=0;
	ogl_extension *cur=NULL;

	for (int i=0; i < GL_NUM_EXTENSIONS; i++)
	{
		cur=&GL_Extensions[i];
		if (opengl_find_extension(cur->extension_name))
		{
			//some extensions do not have functions
			if (cur->function_name==NULL)
			{
				mprintf(("found extension %s\n", cur->extension_name));
				cur->enabled=1;
				num_found++;
				continue;
			}
			
			cur->func_pointer=(uint)wglGetProcAddress(cur->function_name);
			if (cur->func_pointer)
			{
				cur->enabled=1;
				mprintf(("found extension function: %s -- extension: %s\n", cur->function_name, cur->extension_name));
				num_found++;
			}
			else
			{
				mprintf(("found extension, but not function: %s -- extension:%s\n", cur->function_name, cur->extension_name));
			}
		}
		else
		{
			mprintf(("did not find extension: %s\n", cur->extension_name));
			if (cur->required_to_run)
			{
				Error(__FILE__,__LINE__,"The required OpenGL extension %s is not supported by your graphics card, please use the Glide or Direct3D rendering engines.\n\n",cur->extension_name);
			}
		}
	}
	return num_found;
}

void opengl_go_fullscreen(HWND wnd)
{
	DEVMODE dm;

	if (Cmdline_window)
		return;

	mprintf(("opengl_go_fullscreen\n"));

	os_suspend();
	SetWindowLong( wnd, GWL_EXSTYLE, 0 );
	SetWindowLong( wnd, GWL_STYLE, WS_POPUP );
	ShowWindow(wnd, SW_SHOWNORMAL );
	SetWindowPos( wnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	
	SetActiveWindow(wnd);
	SetForegroundWindow(wnd);
	
	memset((void*)&dm,0,sizeof(DEVMODE));
	dm.dmSize=sizeof(DEVMODE);
	dm.dmPelsHeight=gr_screen.max_h;
	dm.dmPelsWidth=gr_screen.max_w;
	dm.dmBitsPerPel=gr_screen.bits_per_pixel;
	dm.dmFields=DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	ChangeDisplaySettings(&dm,CDS_FULLSCREEN);
	os_resume();
}

void opengl_minimize()
{
	HWND wnd=(HWND)os_get_window();
	mprintf(("opengl_minimize\n"));
	
	os_suspend();
	ShowWindow(wnd, SW_MINIMIZE);
	ChangeDisplaySettings(NULL,0);
	os_resume();
}

static inline void opengl_set_max_anistropy()
{
//	if (GL_Extensions[GL_TEX_FILTER_aniso].enabled)		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
}

gr_texture_source	GL_current_tex_src=TEXTURE_SOURCE_NONE;
gr_alpha_blend		GL_current_alpha_blend=ALPHA_BLEND_NONE;
gr_zbuffer_type		GL_current_ztype=ZBUFFER_TYPE_NONE;

void gr_opengl_set_tex_state_combine_arb(gr_texture_source ts)
{
	switch (ts) {
		case TEXTURE_SOURCE_NONE:
		
			//glBindTexture(GL_TEXTURE_2D, 0);
			opengl_switch_arb0(0);
			opengl_switch_arb1(0);
			opengl_switch_arb2(0);
	
			gr_tcache_set(-1, -1, NULL, NULL );
			break;
		
		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
				
			//	opengl_set_max_anistropy();		
		break;
		
		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb0(1);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
			break;
		default:
			return;
	}
}

void gr_opengl_set_tex_state_combine_ext(gr_texture_source ts)
{
	switch (ts) {
		case TEXTURE_SOURCE_NONE:
		
			//glBindTexture(GL_TEXTURE_2D, 0);
			opengl_switch_arb0(0);
			opengl_switch_arb1(0);
			opengl_switch_arb2(0);
	
			gr_tcache_set(-1, -1, NULL, NULL );
			break;
		
		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
				
			//	opengl_set_max_anistropy();		
		break;
		
		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb0(1);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
			break;
		default:
			return;
	}
}


void gr_opengl_set_tex_state_no_combine(gr_texture_source ts)
{
	switch (ts) {
		case TEXTURE_SOURCE_NONE:
		
			//glBindTexture(GL_TEXTURE_2D, 0);
			opengl_switch_arb0(0);
			opengl_switch_arb1(0);
			opengl_switch_arb2(0);
	
			gr_tcache_set(-1, -1, NULL, NULL );
			break;
		
		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//	opengl_set_max_anistropy();
			break;
		
		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb0(1);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			break;
		default:
			return;
	}
	GL_current_tex_src=ts;
}

void gr_opengl_set_additive_tex_env()
{
	if (GL_Extensions[GL_ARB_ENV_COMBINE].enabled)
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
	}
	else if (GL_Extensions[GL_EXT_ENV_COMBINE].enabled)
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
	}
	else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	}
}

void gr_opengl_set_tex_env_scale(float scale)
{
	if (GL_Extensions[GL_ARB_ENV_COMBINE].enabled)
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, scale);
	else if (GL_Extensions[GL_EXT_ENV_COMBINE].enabled)
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, scale);
	else
	{}
}



void gr_opengl_set_state(gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt)
{

	gr_opengl_set_tex_state_combine_arb(ts);


	switch (ab) {
		case ALPHA_BLEND_NONE:
			glBlendFunc(GL_ONE, GL_ZERO);
			break;

		case ALPHA_BLEND_ALPHA_ADDITIVE:
			glBlendFunc(GL_ONE, GL_ONE);
			break;

		case ALPHA_BLEND_ALPHA_BLEND_ALPHA:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		
		case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:
			glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
			break;
	
		default:
			break;
	}
	
	switch (zt) {
		case ZBUFFER_TYPE_NONE:
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_FALSE);
			break;

		case ZBUFFER_TYPE_READ:
			glDepthFunc(GL_LESS);
			glDepthMask(GL_FALSE);	
			break;

		case ZBUFFER_TYPE_WRITE:
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_TRUE);
			break;
	
		case ZBUFFER_TYPE_FULL:
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
			break;
	
		default:
			break;
		}	
}

void gr_opengl_activate(int active)
{
	HWND wnd=(HWND)os_get_window();
	if (active) {
		GL_activate++;
		opengl_go_fullscreen(wnd);

	} else {
		GL_deactivate++;
		opengl_minimize();
	}
	
}


void opengl_tcache_flush ();

void gr_opengl_preload_init()
{
	if (gr_screen.mode != GR_OPENGL) {
		return;
	}

	opengl_tcache_flush ();
}

int GL_should_preload = 0;
int gr_opengl_preload(int bitmap_num, int is_aabitmap)
{
	if ( gr_screen.mode != GR_OPENGL) {
		return 0;
	}

	if ( !GL_should_preload )      {
		return 0;
	}

	float u_scale, v_scale;

	int retval;
	if ( is_aabitmap )      {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale, 1 );
	} else {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 1 );
	}

	if ( !retval )  {
		mprintf(("Texture upload failed!\n" ));
	}

	return retval;
}

void gr_opengl_pixel(int x, int y)
{
	gr_line(x,y,x,y);
}

void gr_opengl_clear()
{
	glClearColor(gr_screen.current_clear_color.red / 255.0f, 
		gr_screen.current_clear_color.green / 255.0f, 
		gr_screen.current_clear_color.blue / 255.0f, 1.0f);

	glClear ( GL_COLOR_BUFFER_BIT );
}

void gr_opengl_save_mouse_area(int x, int y, int w, int h);
void opengl_tcache_frame ();
void gr_opengl_flip()
{
	int swap_error=0;
	if (!OGL_inited) return;

	gr_reset_clip();

	mouse_eval_deltas();
	
	Gr_opengl_mouse_saved = 0;
	
	if ( mouse_is_visible() )       {
		int mx, my;
		
	 	gr_reset_clip();
	 	mouse_get_pos( &mx, &my );
	 	
	 	gr_opengl_save_mouse_area(mx,my,24,24);
	 	
	 	if ( Gr_cursor == -1 )  {
	 		// stuff
	 	} else {
	 		gr_set_bitmap(Gr_cursor);
			gr_bitmap( mx, my );
	 	}
	 }
	 
#ifndef NDEBUG
	GLenum error = glGetError();
	int ic = 0;
	do {
		error = glGetError();
		
		if (error != GL_NO_ERROR) {
			mprintf(("!!DEBUG!! OpenGL Error: %s (%d this frame)\n", gluErrorString(error), ic));
		}
		ic++;
	} while (error != GL_NO_ERROR);

#endif

	TIMERBAR_END_FRAME();
	TIMERBAR_START_FRAME();
	
	if(!SwapBuffers(dev_context))
	{
		opengl_minimize();
		swap_error=GetLastError();
		Error(LOCATION,"Unable to swap buffers\nError code: %d",swap_error);
		exit(2);
	}

	opengl_tcache_frame ();
	
	int cnt = GL_activate;
	if ( cnt )      {
		GL_activate-=cnt;
		opengl_tcache_flush();
		// gr_opengl_clip_cursor(1); /* mouse grab, see opengl_activate */
	}
	
	cnt = GL_deactivate;
	if ( cnt )      {
		GL_deactivate-=cnt;
		// gr_opengl_clip_cursor(0);  /* mouse grab, see opengl_activate */
	}
}

void gr_opengl_flip_window(uint _hdc, int x, int y, int w, int h )
{
	// Not used.
}

void gr_opengl_set_clip(int x,int y,int w,int h)
{

	// check for sanity of parameters
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	if (x >= gr_screen.max_w)
		x = gr_screen.max_w - 1;
	if (y >= gr_screen.max_h)
		y = gr_screen.max_h - 1;

	if (x + w > gr_screen.max_w)
		w = gr_screen.max_w - x;
	if (y + h > gr_screen.max_h)
		h = gr_screen.max_h - y;
	
	if (w > gr_screen.max_w)
		w = gr_screen.max_w;
	if (h > gr_screen.max_h)
		h = gr_screen.max_h;
	
	gr_screen.offset_x = x;
	gr_screen.offset_y = y;
	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;
	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;
	gr_screen.clip_width = w;
	gr_screen.clip_height = h;
	
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, gr_screen.max_h-y-h, w, h);

}

void gr_opengl_reset_clip()
{
	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;

	glDisable(GL_SCISSOR_TEST);
}

void gr_opengl_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;

	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
}

void gr_opengl_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;	
}

void gr_opengl_set_shader( shader * shade )
{	
	if ( shade )	{
		if (shade->screen_sig != gr_screen.signature)	{
			gr_create_shader( shade, shade->r, shade->g, shade->b, shade->c );
		}
		gr_screen.current_shader = *shade;
	} else {
		gr_create_shader( &gr_screen.current_shader, 0.0f, 0.0f, 0.0f, 0.0f );
	}
}

void gr_opengl_rect_internal(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int saved_zbuf;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};

	memset(v,0,sizeof(vertex)*4);
	saved_zbuf = gr_zbuffer_get();
	
	// start the frame, no zbuffering, no culling
#ifndef FRED
	g3_start_frame(1);	
#endif
	gr_zbuffer_set(GR_ZBUFF_NONE);		
	gr_set_cull(0);		

	// stuff coords		
	v[0].sx = i2fl(x);
	v[0].sy = i2fl(y);
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;
	v[0].r = (ubyte)r;
	v[0].g = (ubyte)g;
	v[0].b = (ubyte)b;
	v[0].a = (ubyte)a;

	v[1].sx = i2fl(x + w);
	v[1].sy = i2fl(y);	
	v[1].sw = 0.0f;
	v[1].u = 0.0f;
	v[1].v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;
	v[1].r = (ubyte)r;
	v[1].g = (ubyte)g;
	v[1].b = (ubyte)b;
	v[1].a = (ubyte)a;

	v[2].sx = i2fl(x + w);
	v[2].sy = i2fl(y + h);
	v[2].sw = 0.0f;
	v[2].u = 0.0f;
	v[2].v = 0.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;
	v[2].r = (ubyte)r;
	v[2].g = (ubyte)g;
	v[2].b = (ubyte)b;
	v[2].a = (ubyte)a;

	v[3].sx = i2fl(x);
	v[3].sy = i2fl(y + h);
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 0.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;				
	v[3].r = (ubyte)r;
	v[3].g = (ubyte)g;
	v[3].b = (ubyte)b;
	v[3].a = (ubyte)a;

	// draw the polys
	g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_ALPHA, 0.1f);		

#ifndef FRED
 	g3_end_frame();
#endif

	// restore zbuffer and culling
	gr_zbuffer_set(saved_zbuf);
	gr_set_cull(1);	
}

void gr_opengl_rect(int x,int y,int w,int h)
{
	gr_opengl_rect_internal(x, y, w, h, gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
}

void gr_opengl_shade(int x,int y,int w,int h)
{
	int r,g,b,a;
	
	float shade1 = 1.0f;
	float shade2 = 6.0f;

	r = fl2i(gr_screen.current_shader.r*255.0f*shade1);
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	g = fl2i(gr_screen.current_shader.g*255.0f*shade1);
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	b = fl2i(gr_screen.current_shader.b*255.0f*shade1);
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	a = fl2i(gr_screen.current_shader.c*255.0f*shade2);
	if ( a < 0 ) a = 0; else if ( a > 255 ) a = 255;

        gr_opengl_rect_internal(x, y, w, h, r, g, b, a);	
}

void gr_opengl_aabitmap_ex_internal(int x,int y,int w,int h,int sx,int sy)
{
//	mprintf(("gr_opengl_aabitmap_ex_internal: at (%3d,%3d) size (%3d,%3d) name %s\n", 
  //				x, y, w, h, 
 	//			bm_get_filename(gr_screen.current_bitmap)));
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	if ( !gr_screen.current_color.is_alphacolor )	return;

	float u_scale, v_scale;

	gr_opengl_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		mprintf(( "WARNING: Error setting aabitmap texture!\n" ));
		return;
	}

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh;

	bm_get_info( gr_screen.current_bitmap, &bw, &bh );

	u0 = u_scale*i2fl(sx)/i2fl(bw);
	v0 = v_scale*i2fl(sy)/i2fl(bh);

	u1 = u_scale*i2fl(sx+w)/i2fl(bw);
	v1 = v_scale*i2fl(sy+h)/i2fl(bh);

	x1 = i2fl(x+gr_screen.offset_x);
	y1 = i2fl(y+gr_screen.offset_y);
	x2 = i2fl(x+w+gr_screen.offset_x);
	y2 = i2fl(y+h+gr_screen.offset_y);

	if ( gr_screen.current_color.is_alphacolor )	{
		glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue,gr_screen.current_color.alpha);
		//glColor3ub(255,255,255);
	} else {
		glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
		//glColor3ub(255,255,255);
	}

	ubyte zero[]={0,0,0};
	glSecondaryColor3ubvEXT(zero);

	glBegin (GL_QUADS);
	  glTexCoord2f (u0, v1);
	  glVertex3f (x1, y2, -0.99f);

	  glTexCoord2f (u1, v1);
	  glVertex3f (x2, y2, -0.99f);

	  glTexCoord2f (u1, v0);
	  glVertex3f (x2, y1, -0.99f);

	  glTexCoord2f (u0, v0);
	  glVertex3f (x1, y1, -0.99f);
	glEnd ();

}

void gr_opengl_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	gr_opengl_aabitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_opengl_aabitmap(int x, int y)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );
	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
	if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
	if ( dx1 < gr_screen.clip_left ) { sx = gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
	if ( dy1 < gr_screen.clip_top ) { sy = gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
	if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
	if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	gr_aabitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}


void gr_opengl_string( int sx, int sy, char *s )
{
	TIMERBAR_PUSH(4);
	int width, spacing, letter;
	int x, y;

	if ( !Current_font )	{
		return;
	}

	gr_set_bitmap(Current_font->bitmap_id);

	x = sx;
	y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}
	
	spacing = 0;

	while (*s)	{
		x += spacing;

		while (*s== '\n' )	{
			s++;
			y += Current_font->h;
			if (sx==0x8000) {			//centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}
		if (*s == 0 ) break;

		letter = get_char_width(s[0],s[1],&width,&spacing);
		s++;

		//not in font, draw as space
		if (letter<0)	{
			continue;
		}

		int xd, yd, xc, yc;
		int wc, hc;

		// Check if this character is totally clipped
		if ( x + width < gr_screen.clip_left ) continue;
		if ( y + Current_font->h < gr_screen.clip_top ) continue;
		if ( x > gr_screen.clip_right ) continue;
		if ( y > gr_screen.clip_bottom ) continue;

		xd = yd = 0;
		if ( x < gr_screen.clip_left ) xd = gr_screen.clip_left - x;
		if ( y < gr_screen.clip_top ) yd = gr_screen.clip_top - y;
		xc = x+xd;
		yc = y+yd;

		wc = width - xd; hc = Current_font->h - yd;
		if ( xc + wc > gr_screen.clip_right ) wc = gr_screen.clip_right - xc;
		if ( yc + hc > gr_screen.clip_bottom ) hc = gr_screen.clip_bottom - yc;

		if ( wc < 1 ) continue;
		if ( hc < 1 ) continue;

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		gr_opengl_aabitmap_ex_internal( xc, yc, wc, hc, u+xd, v+yd );
	}

	TIMERBAR_POP();
}

void gr_opengl_line(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
	
	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);
	
	float sx1, sy1;
	float sx2, sy2;
	
	sx1 = i2fl(x1 + gr_screen.offset_x);
	sy1 = i2fl(y1 + gr_screen.offset_y);
	sx2 = i2fl(x2 + gr_screen.offset_x);
	sy2 = i2fl(y2 + gr_screen.offset_y);
	
	if ( x1 == x2 && y1 == y2 ) {
		glBegin (GL_POINTS);
		  glColor4ub (gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
		 
		  ubyte zero[]={0,0,0};
		  glSecondaryColor3ubvEXT(zero);

		  glVertex3f (sx1, sy1, -0.99f);
		glEnd ();
		
		return;
	}
	
	if ( x1 == x2 ) {
		if ( sy1 < sy2 )    {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if ( y1 == y2 )  {
		if ( sx1 < sx2 )    {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}
	
	glBegin (GL_LINES);
	  glColor4ub (gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
	  
	  ubyte zero[]={0,0,0};
	  glSecondaryColor3ubvEXT(zero);

	  glVertex3f (sx2, sy2, -0.99f);
	  glVertex3f (sx1, sy1, -0.99f);
	glEnd ();
}

void gr_opengl_aaline(vertex *v1, vertex *v2)
{
	gr_opengl_line( fl2i(v1->sx), fl2i(v1->sy), fl2i(v2->sx), fl2i(v2->sy) );
}

void gr_opengl_gradient(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	if ( !gr_screen.current_color.is_alphacolor )   {
		gr_line( x1, y1, x2, y2 );
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	int aa = swapped ? 0 : gr_screen.current_color.alpha;
	int ba = swapped ? gr_screen.current_color.alpha : 0;
	
	float sx1, sy1;
	float sx2, sy2;
	
	sx1 = i2fl(x1 + gr_screen.offset_x);
	sy1 = i2fl(y1 + gr_screen.offset_y);
	sx2 = i2fl(x2 + gr_screen.offset_x);
	sy2 = i2fl(y2 + gr_screen.offset_y);
	
	if ( x1 == x2 ) {
		if ( sy1 < sy2 )    {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if ( y1 == y2 )  {
		if ( sx1 < sx2 )    {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	ubyte zero[]={0,0,0};
	glSecondaryColor3ubvEXT(zero);
	
	glBegin (GL_LINES);
	  glColor4ub ((ubyte)gr_screen.current_color.red, (ubyte)gr_screen.current_color.green, (ubyte)gr_screen.current_color.blue, (ubyte)ba);
	  glVertex3f (sx2, sy2, -0.99f);
	  glColor4ub ((ubyte)gr_screen.current_color.red, (ubyte)gr_screen.current_color.green, (ubyte)gr_screen.current_color.blue, (ubyte)aa);
	  glVertex3f (sx1, sy1, -0.99f);
	glEnd ();	
}

void gr_opengl_circle( int xc, int yc, int d )
{
	int p,x, y, r;

	r = d/2;
	p=3-d;
	x=0;
	y=r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) return;
	if ( (xc-r) > gr_screen.clip_right ) return;
	if ( (yc+r) < gr_screen.clip_top ) return;
	if ( (yc-r) > gr_screen.clip_bottom ) return;

	while(x<y)	{
		// Draw the first octant
		gr_opengl_line( xc-y, yc-x, xc+y, yc-x );
		gr_opengl_line( xc-y, yc+x, xc+y, yc+x );
                                
		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			gr_opengl_line( xc-x, yc-y, xc+x, yc-y );
			gr_opengl_line( xc-x, yc+y, xc+x, yc+y );
                                                
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y) {
		gr_opengl_line( xc-x, yc-y, xc+x, yc-y );
		gr_opengl_line( xc-x, yc+y, xc+x, yc+y );
	}
	return;
}


extern vector *Interp_pos;
extern vector Interp_offset;
extern matrix *Interp_orient;

void gr_opengl_stuff_fog_coord(vertex *v)
{
	float d;
	vector pos;			//position of the vertex in question
	vector final;
	vm_vec_add(&pos, Interp_pos,&Interp_offset);
	vm_vec_add2(&pos, &v->real_pos);
	vm_vec_rotate(&final, &pos, Interp_orient);
	d=vm_vec_dist_squared(&pos,&Eye_position);
	glFogCoordfEXT(d);
}

void gr_opengl_stuff_secondary_color(vertex *v, ubyte fr, ubyte fg, ubyte fb)
{

	float color[]={	(float)fr/255.0f,
					(float)fg/255.0f,
					(float)fb/255.0f};

	if ((fr==0) && (fg==0) && (fb==0))
	{
		//easy out
		glSecondaryColor3fvEXT(color);
		return;
	}

	float d;
	float d_over_far;
	vector pos;			//position of the vertex in question
	vm_vec_add(&pos, Interp_pos,&Interp_offset);
	vm_vec_add2(&pos, &v->real_pos);
	d=vm_vec_dist_squared(&pos,&Eye_position);
	
	d_over_far = d/(gr_screen.fog_far*gr_screen.fog_far);

	if (d_over_far <= (gr_screen.fog_near * gr_screen.fog_near))
	{
		memset(color,0,sizeof(float)*3);
		glSecondaryColor3fvEXT(color);
		return;
	}

	if (d_over_far >= 1.0f)
	{
		//another easy out
		glSecondaryColor3fvEXT(color);
		return;
	}

	color[0]*=d_over_far;
	color[1]*=d_over_far;
	color[2]*=d_over_far;

	glSecondaryColor3fvEXT(color);
	return;
}

static int ogl_maybe_pop_arb1=0;

void opengl_draw_primitive(int nv, vertex ** verts, uint flags, float u_scale, float v_scale, int r, int g, int b, int alpha, int override_primary=0)
{

	if (flags & TMAP_FLAG_TRISTRIP) 
		glBegin(GL_TRIANGLE_STRIP);
	else 
		glBegin(GL_TRIANGLE_FAN);

	for (int i = nv-1; i >= 0; i--) {		
		vertex * va = verts[i];
		float sx, sy, sz;
		float tu, tv;
		float rhw;
		int a;
		
		if ( gr_zbuffering || (flags & TMAP_FLAG_NEBULA) )      {
			sz = float(1.0 - 1.0 / (1.0 + va->z / 32768.0 ));
			
			//if ( sz > 0.98f ) {
		//		sz = 0.98f;
		//	}
		} else {
			sz = 0.99f;
		}

		if ( flags & TMAP_FLAG_CORRECT )        {
			rhw = va->sw;
		} else {
			rhw = 1.0f;
		}
		
		if (flags & TMAP_FLAG_ALPHA) {
			a = verts[i]->a;
		} else {
			a = alpha;
		}

		if (flags & TMAP_FLAG_NEBULA ) {
			int pal = (verts[i]->b*(NEBULA_COLORS-1))/255;
			r = gr_palette[pal*3+0];
			g = gr_palette[pal*3+1];
			b = gr_palette[pal*3+2];
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) )   {
			r = Gr_gamma_lookup[verts[i]->b];
			g = Gr_gamma_lookup[verts[i]->b];
			b = Gr_gamma_lookup[verts[i]->b];
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )   {
			// Make 0.75 be 256.0f
			r = Gr_gamma_lookup[verts[i]->r];
			g = Gr_gamma_lookup[verts[i]->g];
			b = Gr_gamma_lookup[verts[i]->b];
		} else {
			// use constant RGB values...
		}

		if (gr_screen.current_bitmap==CLOAKMAP)
		{
			r=g=b=Interp_cloakmap_alpha;
			a=255;
		}

		ubyte sc[]={(ubyte)(va->spec_r >> 1), (ubyte)(va->spec_g >> 1), (ubyte)(va->spec_b >> 1)};

		if (!override_primary)
		{
			glColor4ub ((ubyte)r,(ubyte)g,(ubyte)b,(ubyte)a);
			glSecondaryColor3ubvEXT(sc);
		}
		else
		{
			glColor3ubv(sc);		
			glSecondaryColor3ubvEXT(sc);
		}
		

			
		if((gr_screen.current_fog_mode != GR_FOGMODE_NONE) && (OGL_fogmode == 2)){
		
			// this is for GL_EXT_FOG_COORD 
			gr_opengl_stuff_fog_coord(va);
		}

		int x, y;
		x = fl2i(va->sx*16.0f);
		y = fl2i(va->sy*16.0f);

		x += gr_screen.offset_x*16;
		y += gr_screen.offset_y*16;
		
		sx = i2fl(x) / 16.0f;
		sy = i2fl(y) / 16.0f;

		if ( flags & TMAP_FLAG_TEXTURED )       {
			tu = va->u*u_scale;
			tv = va->v*v_scale;

			//use opengl hardware multitexturing
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,tu,tv);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,tu,tv);
			
			if (max_multitex>2)			glMultiTexCoord2fARB(GL_TEXTURE2_ARB,tu,tv);
			
		}
		
		glVertex4f(sx/rhw, sy/rhw, -sz/rhw, 1.0f/rhw);
	}
	glEnd();
}

void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale )
{
	gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
	GLOWMAP=-1;

	if ( !gr_tcache_set(SPECMAP, tmap_type, u_scale, v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))
	{
		//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
		return;
	}

	glBlendFunc(GL_ONE,GL_ONE);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	gr_opengl_set_tex_env_scale(4.0f);
	
}

void opengl_reset_spec_mapping()
{
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler)
{
	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;
	
	if ( gr_zbuffering )    {
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)   )       {
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}
	
	tmap_type = TCACHE_TYPE_NORMAL;

	if ( flags & TMAP_FLAG_TEXTURED )       {
		r = g = b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
//		opengl_switch_arb0(0);
//		opengl_switch_arb1(0);
	}

	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )        
	{
		if (1) {
			tmap_type = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;
			
			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;
			
			alpha = 255;
			
			if ( factor <= 1.0f )   {
				int tmp_alpha = fl2i(gr_screen.current_alpha*255.0f);
				r = (r*tmp_alpha)/255;
				g = (g*tmp_alpha)/255;
				b = (b*tmp_alpha)/255;
			}
		} else {
			tmap_type = TCACHE_TYPE_XPARENT;
			
			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
			
			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;
				
			if ( factor > 1.0f )    {
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
	} else {
		if(Bm_pixel_format == BM_PIXEL_FORMAT_ARGB) {
			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		} else {
			alpha_blend = ALPHA_BLEND_NONE;
		}
		alpha = 255;
	}

	if(flags & TMAP_FLAG_BITMAP_SECTION){
		tmap_type = TCACHE_TYPE_BITMAP_SECTION;
	}
	
	texture_source = TEXTURE_SOURCE_NONE;
	
	if ( flags & TMAP_FLAG_TEXTURED )       {
		texture_source=TEXTURE_SOURCE_DECAL;
			
		// use nonfiltered textures for bitmap sections
		if(flags & TMAP_FLAG_BITMAP_SECTION){
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	}

	gr_opengl_set_state( texture_source, alpha_blend, zbuffer_type );
}

void gr_opengl_tmapper_internal_2multitex( int nv, vertex ** verts, uint flags, int is_scaler )
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3 ();
	}

	int alpha,tmap_type, r, g, b;

	opengl_setup_render_states(r,g,b,alpha,tmap_type,flags,is_scaler);


	if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))
	{
		//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
		return;
	}
	
	if (flags & TMAP_FLAG_PIXEL_FOG) {
		int r, g, b;
		int ra, ga, ba;
		ra = ga = ba = 0;
	
		for (i=nv-1;i>=0 ;i--)	
		{
			vertex *va = verts[i];
			float sx, sy;
                
			int x, y;
			x = fl2i(va->sx*16.0f);
			y = fl2i(va->sy*16.0f);

			x += gr_screen.offset_x*16;
			y += gr_screen.offset_y*16;

			sx = i2fl(x) / 16.0f;
			sy = i2fl(y) / 16.0f;

			neb2_get_pixel((int)sx, (int)sy, &r, &g, &b);
			
			ra += r;
			ga += g;
			ba += b;
		}
		
		ra /= nv;
		ga /= nv;
		ba /= nv;

		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}

	if (CLOAKMAP==gr_screen.current_bitmap)
		glBlendFunc(GL_ONE, GL_ONE);

	opengl_draw_primitive(nv,verts,flags,u_scale,v_scale,r,g,b,alpha);

	//if we used arb1 for the cloakmap, return
	if (ogl_maybe_pop_arb1)
	{
		gr_pop_texture_matrix(1);
		ogl_maybe_pop_arb1=0;
		return;
	}

	if (Interp_multitex_cloakmap==1)
	{
	
		GLOWMAP=-1;

		gr_screen.gf_set_bitmap(CLOAKMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);

		//maybe draw a cloakmap, using a multipass technique
		if ( !gr_tcache_set(CLOAKMAP, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))
		{
			//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glTranslated(tex_shift.xyz.x,tex_shift.xyz.y, tex_shift.xyz.z);
		glMatrixMode(GL_MODELVIEW);


		glBlendFunc(GL_ZERO,GL_SRC_COLOR);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
	
		opengl_draw_primitive(nv,verts,flags,u_scale,v_scale,r,g,b,alpha);


		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
	}

	//maybe do a spec map
	if (SPECMAP > -1)
	{
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		GLOWMAP=-1;

		if ( !gr_tcache_set(SPECMAP, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))
		{
			//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		glBlendFunc(GL_ONE,GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		gr_opengl_set_tex_env_scale(4.0f);
		opengl_draw_primitive(nv,verts,flags,u_scale,v_scale,r,g,b,1);
			
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
}

void gr_opengl_tmapper_internal_3multitex( int nv, vertex ** verts, uint flags, int is_scaler )
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3 ();
	}

	
	int alpha,tmap_type, r, g, b;

	opengl_setup_render_states(r,g,b,alpha,tmap_type,flags,is_scaler);

	
	if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))
	{
		//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
		return;
	}
	
	
	if (flags & TMAP_FLAG_PIXEL_FOG) {
		int r, g, b;
		int ra, ga, ba;
		ra = ga = ba = 0;
	
		for (i=nv-1;i>=0 ;i--)	
		{
			vertex *va = verts[i];
			float sx, sy;
                
			int x, y;
			x = fl2i(va->sx*16.0f);
			y = fl2i(va->sy*16.0f);

			x += gr_screen.offset_x*16;
			y += gr_screen.offset_y*16;

			sx = i2fl(x) / 16.0f;
			sy = i2fl(y) / 16.0f;

			neb2_get_pixel((int)sx, (int)sy, &r, &g, &b);
			
			ra += r;
			ga += g;
			ba += b;
		}
		
		ra /= nv;
		ga /= nv;
		ba /= nv;

		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}

	if (CLOAKMAP==gr_screen.current_bitmap)
		glBlendFunc(GL_ONE, GL_ONE);

	opengl_draw_primitive(nv,verts,flags, u_scale, v_scale, r,g,b,alpha);

	if (ogl_maybe_pop_arb1)
	{
		gr_pop_texture_matrix(1);
		ogl_maybe_pop_arb1=0;
	}

	if ((SPECMAP > -1) && (flags & TMAP_FLAG_TEXTURED))
	{
		opengl_set_spec_mapping(tmap_type,&u_scale,&v_scale);
		opengl_draw_primitive(nv,verts,flags,u_scale,v_scale,r,g,b,alpha,1);
		opengl_reset_spec_mapping();
	}
}


//ok we're making some assumptions for now.  mainly it must not be multitextured, lit or fogged.
void gr_opengl_tmapper_internal3d( int nv, vertex ** verts, uint flags, int is_scaler )
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3 ();
	}
		
	glDisable(GL_CULL_FACE);
	
	int alpha,tmap_type, r, g, b;

	opengl_setup_render_states(r,g,b,alpha,tmap_type,flags,is_scaler);

	if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))
	{
		mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
		return;
	}



	glColor3ub(191,191,191);	//its unlit

	vertex *va;
	if (flags & TMAP_FLAG_TRISTRIP) glBegin(GL_TRIANGLE_STRIP);
	else glBegin(GL_TRIANGLE_FAN);
	for (i=0; i < nv; i++)
	{
		va=verts[i];
		if(flags & TMAP_FLAG_RGB) glColor3ub(Gr_gamma_lookup[va->r],Gr_gamma_lookup[va->g],Gr_gamma_lookup[va->b]);
		glTexCoord2f(va->u, va->v);
		glVertex3f(va->x,va->y,va->z);
	}
	glEnd();

}


void gr_opengl_tmapper( int nverts, vertex **verts, uint flags )
{
	if (flags & TMAP_HTL_3D_UNLIT)
		gr_opengl_tmapper_internal3d( nverts, verts, flags, 0 );
	else
		gr_opengl_tmapper_internal(nverts,verts,flags,0);

}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

void gr_opengl_scaler(vertex *va, vertex *vb )
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	//============= CLIP IT =====================

	x0 = va->sx; y0 = va->sy;
	x1 = vb->sx; y1 = vb->sy;

	xmin = i2fl(gr_screen.clip_left); ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right); ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->u; v0 = va->v;
	u1 = vb->u; v1 = vb->v;

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<xmin ) || (x0>xmax) ) return;
	if ( (y1<ymin ) || (y0>ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = fl2i(clipped_x0); dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0); dy1 = fl2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	//============= DRAW IT =====================

	vertex v[4];
	vertex *vl[4];

	vl[0] = &v[0];	
	v->sx = clipped_x0;
	v->sy = clipped_y0;
	v->sw = va->sw;
	v->z = va->z;
	v->u = clipped_u0;
	v->v = clipped_v0;
	v->spec_r=0;
	v->spec_g=0;
	v->spec_b=0;

	vl[1] = &v[1];	
	v[1].sx = clipped_x1;
	v[1].sy = clipped_y0;
	v[1].sw = va->sw;
	v[1].z = va->z;
	v[1].u = clipped_u1;
	v[1].v = clipped_v0;
	v[1].spec_r=0;
	v[1].spec_g=0;
	v[1].spec_b=0;

	vl[2] = &v[2];	
	v[2].sx = clipped_x1;
	v[2].sy = clipped_y1;
	v[2].sw = va->sw;
	v[2].z = va->z;
	v[2].u = clipped_u1;
	v[2].v = clipped_v1;
	v[2].spec_r=0;
	v[2].spec_g=0;
	v[2].spec_b=0;

	vl[3] = &v[3];	
	v[3].sx = clipped_x0;
	v[3].sy = clipped_y1;
	v[3].sw = va->sw;
	v[3].z = va->z;
	v[3].u = clipped_u0;
	v[3].v = clipped_v1;
	v[3].spec_r=0;
	v[3].spec_g=0;
	v[3].spec_b=0;

	gr_opengl_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
}

void gr_opengl_set_palette(ubyte *new_palette, int is_alphacolor)
{
}

void gr_opengl_get_color( int * r, int * g, int * b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

void gr_opengl_init_color(color *c, int r, int g, int b)
{
	c->screen_sig = gr_screen.signature;
	c->red = (unsigned char)r;
	c->green = (unsigned char)g;
	c->blue = (unsigned char)b;
	c->alpha = 255;
	c->ac_type = AC_TYPE_NONE;
	c->alphacolor = -1;
	c->is_alphacolor = 0;
	c->magic = 0xAC01;
}

void gr_opengl_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( alpha < 0 ) alpha = 0; else if ( alpha > 255 ) alpha = 255;

        gr_opengl_init_color( clr, r, g, b );

        clr->alpha = (unsigned char)alpha;
        clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

void gr_opengl_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_opengl_init_color( &gr_screen.current_color, r, g, b );	
}

void gr_opengl_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )       {
			gr_opengl_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_opengl_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}
	gr_screen.current_color = *dst;
}

void gr_opengl_print_screen(char *filename)
{
	ubyte buf[1024*3];

	memset(buf,0,1024*3);
	char tmp[1024];

	strcpy( tmp, NOX(".\\gl"));	// specify a path mean files goes in root
	strcat( tmp, filename );
	strcat( tmp, NOX(".tga"));

	CFILE *f = cfopen(tmp, "wb");

	// Write the TGA header
	cfwrite_ubyte( 0, f );	//	IDLength;
	cfwrite_ubyte( 0, f );	//	ColorMapType;
	cfwrite_ubyte( 2, f );	//	ImageType;		// 2 = 24bpp, uncompressed, 10=24bpp rle compressed
	cfwrite_ushort( 0, f );	// CMapStart;
	cfwrite_ushort( 0, f );	//	CMapLength;
	cfwrite_ubyte( 0, f );	// CMapDepth;
	cfwrite_ushort( 0, f );	//	XOffset;
	cfwrite_ushort( 0, f );	//	YOffset;
	cfwrite_ushort( (ushort)gr_screen.max_w, f );	//	Width;
	cfwrite_ushort( (ushort)gr_screen.max_h, f );	//	Height;
	cfwrite_ubyte( 24, f );	//PixelDepth;
	cfwrite_ubyte( 0, f );	//ImageDesc;
	
	int h=gr_screen.max_h;
	int w=gr_screen.max_w;

	for (int i=0; i < h; i++)
	{
		glReadPixels(0,i,w,1,GL_BGR_EXT, GL_UNSIGNED_BYTE, buf);	
		cfwrite(buf,w*3,1,f);
	}
	
	cfclose(f);

}

int gr_opengl_supports_res_ingame(int res)
{
	STUB_FUNCTION;
	
	return 1;
}

int gr_opengl_supports_res_interface(int res)
{
	STUB_FUNCTION;
	
	return 1;
}

void opengl_tcache_cleanup ();
void gr_opengl_cleanup(int minimize)
{	
	HWND wnd=(HWND)os_get_window();
	if ( !OGL_inited )	return;


	gr_reset_clip();
	gr_clear();
	gr_flip();


	if ( !OGL_inited )	return;
	
	gr_reset_clip();
	gr_clear();
	gr_flip();

	OGL_inited = 0;

	if (rend_context)
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			MessageBox(wnd, "SHUTDOWN ERROR", "error", MB_OK);
		}
		if (!wglDeleteContext(rend_context))
		{
			MessageBox(wnd, "Unable to delete rendering context", "error", MB_OK);
		}
		rend_context=NULL;
	}

	opengl_minimize();
	if (minimize)
	{
		if (!Cmdline_window)
			ChangeDisplaySettings(NULL, 0);
	}

}

void gr_opengl_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
//	mprintf(("gr_opengl_fog_set(%d,%d,%d,%d,%f,%f)\n",fog_mode,r,g,b,fog_near,fog_far));

	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));
	
	if (fog_mode == GR_FOGMODE_NONE) {
		if (gr_screen.current_fog_mode != fog_mode) {
			glDisable(GL_FOG);
		}
		gr_screen.current_fog_mode = fog_mode;
		
		return;
	}
	
	if (gr_screen.current_fog_mode != fog_mode) {
		if (OGL_fogmode==3)
			glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
		else if (OGL_fogmode==2)
		{
			glFogf(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
			fog_near*=fog_near;		//its faster this way
			fog_far*=fog_far;		
		}
		glEnable(GL_FOG);
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, fog_near);
		glFogf(GL_FOG_END, fog_far);
		gr_screen.current_fog_mode = fog_mode;
		

	}
	
	if ( (gr_screen.current_fog_color.red != r) ||
			(gr_screen.current_fog_color.green != g) ||
			(gr_screen.current_fog_color.blue != b) ) {
		GLfloat fc[4];
		
		gr_opengl_init_color( &gr_screen.current_fog_color, r, g, b );
	
		fc[0] = (float)r/255.0f;
		fc[1] = (float)g/255.0f;
		fc[2] = (float)b/255.0f;
		fc[3] = 1.0f;
		
		glFogfv(GL_FOG_COLOR, fc);
	}

}

void gr_opengl_get_pixel(int x, int y, int *r, int *g, int *b)
{
	// Not used.
}

void gr_opengl_set_cull(int cull)
{
	if (cull) {
		glEnable (GL_CULL_FACE);
		glFrontFace (GL_CCW);
	} else {
		glDisable (GL_CULL_FACE);
	}
}

void gr_opengl_filter_set(int filter)
{
}

// cross fade
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	if ( pct <= 50 ) {
		gr_set_bitmap(bmap1);
		gr_bitmap(x1, y1);
	} else {
		gr_set_bitmap(bmap2);
		gr_bitmap(x2, y2);
	}		
}


typedef struct tcache_slot_opengl {
	GLuint	texture_handle;
	float	u_scale, v_scale;
	int	bitmap_id;
	int	size;
	char	used_this_frame;
	int	time_created;
	ushort	w,h;

	// sections
	tcache_slot_opengl	*data_sections[MAX_BMAP_SECTIONS_X][MAX_BMAP_SECTIONS_Y];
	tcache_slot_opengl	*parent;
} tcache_slot_opengl;

static void *Texture_sections = NULL;
static tcache_slot_opengl *Textures = NULL;

int GL_texture_sections = 0;
int GL_texture_ram = 0;
int GL_frame_count = 0;
int GL_min_texture_width = 0;
int GL_max_texture_width = 0;
int GL_min_texture_height = 0;
int GL_max_texture_height = 0;
int GL_square_textures = 0;
int GL_textures_in = 0;
int GL_textures_in_frame = 0;
int GL_last_bitmap_id = -1;
int GL_last_detail = -1;
int GL_last_bitmap_type = -1;
int GL_last_section_x = -1;
int GL_last_section_y = -1;

extern int vram_full;

void opengl_tcache_init (int use_sections)
{
	int i, idx, s_idx;

	// DDOI - FIXME skipped a lot of stuff here
	GL_should_preload = 0;

	//uint tmp_pl = os_config_read_uint( NULL, NOX("D3DPreloadTextures"), 255 );
	uint tmp_pl = 1;

	if ( tmp_pl == 0 )      {
		GL_should_preload = 0;
	} else if ( tmp_pl == 1 )       {
		GL_should_preload = 1;
	} else {
		GL_should_preload = 1;
	}

	GL_min_texture_width = 16;
	GL_min_texture_height = 16;
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GL_max_texture_width);

	GL_max_texture_height=GL_max_texture_width;

	mprintf(("max texture size is: %dx%d\n", GL_max_texture_width,GL_max_texture_height));

	GL_square_textures = 0;

	Textures = (tcache_slot_opengl *)malloc(MAX_BITMAPS*sizeof(tcache_slot_opengl));
	if ( !Textures )        {
		exit(1);
	}

	if(use_sections){
		Texture_sections = (tcache_slot_opengl*)malloc(MAX_BITMAPS * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y * sizeof(tcache_slot_opengl));
		if(!Texture_sections){
			exit(1);
		}
		memset(Texture_sections, 0, MAX_BITMAPS * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y * sizeof(tcache_slot_opengl));
	}

	// Init the texture structures
	int section_count = 0;
	for( i=0; i<MAX_BITMAPS; i++ )  {
		/*
		Textures[i].vram_texture = NULL;
		Textures[i].vram_texture_surface = NULL;
		*/
		Textures[i].texture_handle = 0;

		Textures[i].bitmap_id = -1;
		Textures[i].size = 0;
		Textures[i].used_this_frame = 0;

		Textures[i].parent = NULL;

		// allocate sections
		if(use_sections){
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					Textures[i].data_sections[idx][s_idx] = &((tcache_slot_opengl*)Texture_sections)[section_count++];
					Textures[i].data_sections[idx][s_idx]->parent = &Textures[i];
					/*
					Textures[i].data_sections[idx][s_idx]->vram_texture = NULL;
					Textures[i].data_sections[idx][s_idx]->vram_texture_surface = NULL;
					*/
					Textures[i].data_sections[idx][s_idx]->texture_handle = 0;
					Textures[i].data_sections[idx][s_idx]->bitmap_id = -1;
					Textures[i].data_sections[idx][s_idx]->size = 0;
					Textures[i].data_sections[idx][s_idx]->used_this_frame = 0;
				}
			}
		} else {
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					Textures[i].data_sections[idx][s_idx] = NULL;
				}
			}
		}
	}

	GL_texture_sections = use_sections;

	//GL_last_detail = Detail.hardware_textures;
	GL_last_bitmap_id = -1;
	GL_last_bitmap_type = -1;

	GL_last_section_x = -1;
	GL_last_section_y = -1;

	GL_textures_in = 0;
	GL_textures_in_frame = 0;
}

int opengl_free_texture (tcache_slot_opengl *t);

void opengl_free_texture_with_handle(int handle)
{
	for(int i=0; i<MAX_BITMAPS; i++ )  {
		if (Textures[i].bitmap_id == handle) {
			Textures[i].used_this_frame = 0; // this bmp doesn't even exist any longer...
			opengl_free_texture ( &Textures[i] );
		}
	}
}

void opengl_tcache_flush ()
{
	int i;

	for( i=0; i<MAX_BITMAPS; i++ )  {
		opengl_free_texture ( &Textures[i] );
	}
	if (GL_textures_in != 0) {
		mprintf(( "WARNING: VRAM is at %d instead of zero after flushing!\n", GL_textures_in ));
		GL_textures_in = 0;
	}

	GL_last_bitmap_id = -1;
	GL_last_section_x = -1;
	GL_last_section_y = -1;
}

void opengl_tcache_cleanup ()
{
	opengl_tcache_flush ();

	GL_textures_in = 0;
	GL_textures_in_frame = 0;

	if ( Textures ) {
		free(Textures);
		Textures = NULL;
	}

	if( Texture_sections != NULL ){
		free(Texture_sections);
		Texture_sections = NULL;
	}
}

void opengl_tcache_frame ()
{
	int idx, s_idx;

	GL_last_bitmap_id = -1;
	GL_textures_in_frame = 0;

	GL_frame_count++;

	int i;
	for( i=0; i<MAX_BITMAPS; i++ )  {
		Textures[i].used_this_frame = 0;

		// data sections
		if(Textures[i].data_sections[0][0] != NULL){
			Assert(GL_texture_sections);
			if(GL_texture_sections){
				for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
					for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
						if(Textures[i].data_sections[idx][s_idx] != NULL){
							Textures[i].data_sections[idx][s_idx]->used_this_frame = 0;
						}
					}
				}
			}
		}
	}

	if ( vram_full )        {
		opengl_tcache_flush();
		vram_full = 0;
	}
}

int opengl_free_texture ( tcache_slot_opengl *t )
{
	int idx, s_idx;
	

	// Bitmap changed!!     
	if ( t->bitmap_id > -1 )        {
		// if I, or any of my children have been used this frame, bail  
		if(t->used_this_frame){
			return 0;
		}
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				if((t->data_sections[idx][s_idx] != NULL) && (t->data_sections[idx][s_idx]->used_this_frame)){
					return 0;
				}
			}
		}

		// ok, now we know its legal to free everything safely
		glDeleteTextures (1, &t->texture_handle);
		t->texture_handle = 0;

		if ( GL_last_bitmap_id == t->bitmap_id )       {
			GL_last_bitmap_id = -1;
		}

		// if this guy has children, free them too, since the children
		// actually make up his size
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				if(t->data_sections[idx][s_idx] != NULL){
					opengl_free_texture(t->data_sections[idx][s_idx]);
				}
			}
		}

		t->bitmap_id = -1;
		t->used_this_frame = 0;
		GL_textures_in -= t->size;
	}

	return 1;
}

void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	int tex_w, tex_h;

	// bogus
	if((w_out == NULL) ||  (h_out == NULL)){
		return;
	}

	// starting size
	tex_w = w_in;
	tex_h = h_in;

	if (1)        {
		int i;
		for (i=0; i<16; i++ )   {
			if ( (tex_w > (1<<i)) && (tex_w <= (1<<(i+1))) )        {
				tex_w = 1 << (i+1);
				break;
			}
		}

		for (i=0; i<16; i++ )   {
			if ( (tex_h > (1<<i)) && (tex_h <= (1<<(i+1))) )        {
				tex_h = 1 << (i+1);
				break;
			}
		}
	}

	if ( tex_w < GL_min_texture_width ) {
		tex_w = GL_min_texture_width;
	} else if ( tex_w > GL_max_texture_width )     {
		tex_w = GL_max_texture_width;
	}

	if ( tex_h < GL_min_texture_height ) {
		tex_h = GL_min_texture_height;
	} else if ( tex_h > GL_max_texture_height )    {
		tex_h = GL_max_texture_height;
	}

	if ( GL_square_textures )      {
		int new_size;
		// Make the both be equal to larger of the two
		new_size = max(tex_w, tex_h);
		tex_w = new_size;
		tex_h = new_size;
	}

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

// data == start of bitmap data
// sx == x offset into bitmap
// sy == y offset into bitmap
// src_w == absolute width of section on source bitmap
// src_h == absolute height of section on source bitmap
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int opengl_create_texture_sub(int bitmap_type, int texture_handle, ushort *data, int sx, int sy, int src_w, int src_h, int bmap_w, int bmap_h, int tex_w, int tex_h, tcache_slot_opengl *t, int reload, int fail_on_full)
{
	int ret_val = 1;

	// bogus
	if(t == NULL){
		return 0;
	}

	if ( t->used_this_frame )       {
		mprintf(( "ARGHH!!! Texture already used this frame!  Cannot free it!\n" ));
		return 0;
	}
	if ( !reload )  {
		// gah
		if(!opengl_free_texture(t)){
			return 0;
		}
	}

	// get final texture size
	opengl_tcache_get_adjusted_texture_size(tex_w, tex_h, &tex_w, &tex_h);

	if ( (tex_w < 1) || (tex_h < 1) )       {
		mprintf(("Bitmap is to small at %dx%d.\n", tex_w, tex_h ));
		return 0;
	}

	if ( bitmap_type == TCACHE_TYPE_AABITMAP )      {
		t->u_scale = (float)bmap_w / (float)tex_w;
		t->v_scale = (float)bmap_h / (float)tex_h;
	} else if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		t->u_scale = (float)src_w / (float)tex_w;
		t->v_scale = (float)src_h / (float)tex_h;
	} else {
		t->u_scale = 1.0f;
		t->v_scale = 1.0f;
	}

	if (!reload) {
		glGenTextures (1, &t->texture_handle);
	}
	
	if (t->texture_handle == 0) {
		nprintf(("Error", "!!DEBUG!! t->texture_handle == 0"));
		return 0;
	}
	
	glBindTexture (GL_TEXTURE_2D, t->texture_handle);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	//compression takes precedence
	if (bitmap_type & TCACHE_TYPE_COMPRESSED)
	{
		GLenum ctype(0);
		switch (bm_is_compressed(texture_handle))
		{
			case 1:
				ctype=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				break;

			case 2:
				ctype=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				break;

			case 3:
				ctype=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;

			default:
				Assert(0);
		}
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, ctype, tex_w, tex_h,0, bm_get_size(texture_handle), (ubyte*)data);
	}
	else
	{

		switch (bitmap_type) {
	
			case TCACHE_TYPE_AABITMAP:
			{
				int i,j;
				ubyte *bmp_data = ((ubyte*)data);
				ubyte *texmem = (ubyte *) malloc (tex_w*tex_h*2);
				ubyte *texmemp = texmem;
				ubyte xlat[256];
			
				for (i=0; i<16; i++) {
					xlat[i] = (ubyte)Gr_gamma_lookup[(i*255)/15];
				}	
				xlat[15] = xlat[1];
				for ( ; i<256; i++ )    {
					xlat[i] = xlat[0];
				}
			
				for (i=0;i<tex_h;i++)
				{
					for (j=0;j<tex_w;j++)
					{
					if (i < bmap_h && j < bmap_w) {
							*texmemp++ = 0xff;
							*texmemp++ = xlat[bmp_data[i*bmap_w+j]];
						} else {
							*texmemp++ = 0;
							*texmemp++ = 0;
						}
					}
				}

				glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, tex_w, tex_h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texmem);
				free (texmem);
			}
			break;


		case TCACHE_TYPE_BITMAP_SECTION:
			{
				int i,j;
				ubyte *bmp_data = ((ubyte*)data);
				ubyte *texmem = (ubyte *) malloc (tex_w*tex_h*2);
				ubyte *texmemp = texmem;
				
				for (i=0;i<tex_h;i++)
				{
					for (j=0;j<tex_w;j++)
					{
						if (i < src_h && j < src_w) {
							*texmemp++ = bmp_data[((i+sy)*bmap_w+(j+sx))*2+0];
							*texmemp++ = bmp_data[((i+sy)*bmap_w+(j+sx))*2+1];
						} else {
							*texmemp++ = 0;
							*texmemp++ = 0;
						}
					}
				}
				glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_BGRA,GL_UNSIGNED_SHORT_1_5_5_5_REV, texmem);
				free(texmem);
				break;
			}
		default:
			{
				int i,j;
				ubyte *bmp_data = ((ubyte*)data);
				ubyte *texmem = (ubyte *) malloc (tex_w*tex_h*2);
				ubyte *texmemp = texmem;
				
				fix u, utmp, v, du, dv;
				
				u = v = 0;
				
				du = ( (bmap_w-1)*F1_0 ) / tex_w;
				dv = ( (bmap_h-1)*F1_0 ) / tex_h;
				
				for (j=0;j<tex_h;j++)
				{
					utmp = u;
					for (i=0;i<tex_w;i++)
					{
						*texmemp++ = bmp_data[(f2i(v)*bmap_w+f2i(utmp))*2+0];
						*texmemp++ = bmp_data[(f2i(v)*bmap_w+f2i(utmp))*2+1];
						utmp += du;
					}
					v += dv;
				}

				glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_BGRA,GL_UNSIGNED_SHORT_1_5_5_5_REV, texmem);
				free(texmem);
				break;
			}
		}//end switch
	}//end else
	
	t->bitmap_id = texture_handle;
	t->time_created = GL_frame_count;
	t->used_this_frame = 0;
	if (bitmap_type & TCACHE_TYPE_COMPRESSED) t->size=bm_get_size(texture_handle);
	else	t->size = tex_w * tex_h * 2;
	t->w = (ushort)tex_w;
	t->h = (ushort)tex_h;
	GL_textures_in_frame += t->size;
	if (!reload) {
		GL_textures_in += t->size;
	}

	return ret_val;
}

int opengl_create_texture (int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	ubyte bpp = 16;
	int reload = 0;

	// setup texture/bitmap flags
	flags = 0;
	switch(bitmap_type){
		case TCACHE_TYPE_AABITMAP:
			flags |= BMP_AABITMAP;
			bpp = 8;
			break;
		case TCACHE_TYPE_NORMAL:
			flags |= BMP_TEX_OTHER;
		case TCACHE_TYPE_XPARENT:
			flags |= BMP_TEX_XPARENT;
			break;
		case TCACHE_TYPE_NONDARKENING:
			Int3();
			flags |= BMP_TEX_NONDARK;
			break;
	}
	
	switch (bm_is_compressed(bitmap_handle))
	{
		case 1:				//dxt1
			bpp=24;
			flags |=BMP_TEX_DXT1;
			bitmap_type|=TCACHE_TYPE_COMPRESSED;
			break;

		case 2:				//dxt3
			bpp=32;
			flags |=BMP_TEX_DXT3;
			bitmap_type|=TCACHE_TYPE_COMPRESSED;
			break;

		case 3:				//dxt5
			bpp=32;
			flags |=BMP_TEX_DXT5;
			bitmap_type|=TCACHE_TYPE_COMPRESSED;
			break;
		
		default:
			break;
	}

	// lock the bitmap into the proper format
	bmp = bm_lock(bitmap_handle, bpp, flags);
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}

	int max_w = bmp->w;
	int max_h = bmp->h;

	
	   // DDOI - TODO
	if ( bitmap_type != TCACHE_TYPE_AABITMAP )      {
		// max_w /= D3D_texture_divider;
		// max_h /= D3D_texture_divider;

		// Detail.debris_culling goes from 0 to 4.
		max_w /= (16 >> Detail.hardware_textures);
		max_h /= (16 >> Detail.hardware_textures);
	}
	

	// get final texture size as it will be allocated as a DD surface
	opengl_tcache_get_adjusted_texture_size(max_w, max_h, &final_w, &final_h); 

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_id != bitmap_handle)     {
		if((final_w == tslot->w) && (final_h == tslot->h)){
			reload = 1;
			//ml_printf("Reloading texture %d\n", bitmap_handle);
		} else {
			reload = 0;
		}
	}

	// call the helper
	int ret_val = opengl_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, 0, 0, bmp->w, bmp->h, bmp->w, bmp->h, max_w, max_h, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

int opengl_create_texture_sectioned(int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot, int sx, int sy, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	int section_x, section_y;
	int reload = 0;

	// setup texture/bitmap flags
	Assert(bitmap_type == TCACHE_TYPE_BITMAP_SECTION);
	if(bitmap_type != TCACHE_TYPE_BITMAP_SECTION){
		bitmap_type = TCACHE_TYPE_BITMAP_SECTION;
	}
	flags = BMP_TEX_XPARENT;

	// lock the bitmap in the proper format
	bmp = bm_lock(bitmap_handle, 16, flags);
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}
	// determine the width and height of this section
	bm_get_section_size(bitmap_handle, sx, sy, &section_x, &section_y);

	// get final texture size as it will be allocated as an opengl texture
	opengl_tcache_get_adjusted_texture_size(section_x, section_y, &final_w, &final_h);

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_id != bitmap_handle)     {
		if((final_w == tslot->w) && (final_h == tslot->h)){
			reload = 1;
		} else {
			reload = 0;
		}
	}

	// call the helper
	int ret_val = opengl_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, bmp->sections.sx[sx], bmp->sections.sy[sy], section_x, section_y, bmp->w, bmp->h, section_x, section_y, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

int gr_opengl_tcache_set_internal(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0, int tex_unit = 0)
{
	bitmap *bmp = NULL;

	int idx, s_idx;
	int ret_val = 1;

	if ( GL_last_detail != Detail.hardware_textures )      {
		GL_last_detail = Detail.hardware_textures;
		opengl_tcache_flush();
	}

	if (vram_full) {
		return 0;
	}

	int n = bm_get_cache_slot (bitmap_id, 1);
	tcache_slot_opengl *t = &Textures[n];

	if ( (GL_last_bitmap_id == bitmap_id) && (GL_last_bitmap_type==bitmap_type) && (t->bitmap_id == bitmap_id) && (GL_last_section_x == sx) && (GL_last_section_y == sy))       {
		t->used_this_frame++;

		// mark all children as used
		if(GL_texture_sections){
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					if(t->data_sections[idx][s_idx] != NULL){
						t->data_sections[idx][s_idx]->used_this_frame++;
					}
				}
			}
		}

		*u_scale = t->u_scale;
		*v_scale = t->v_scale;
		return 1;
	}


	glActiveTextureARB(GL_TEXTURE0_ARB+tex_unit);

	opengl_set_max_anistropy();

	if (bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		Assert((sx >= 0) && (sy >= 0) && (sx < MAX_BMAP_SECTIONS_X) && (sy < MAX_BMAP_SECTIONS_Y));
		if(!((sx >= 0) && (sy >= 0) && (sx < MAX_BMAP_SECTIONS_X) && (sy < MAX_BMAP_SECTIONS_Y))){
			return 0;
		}

		ret_val = 1;

		// if the texture sections haven't been created yet
		if((t->bitmap_id < 0) || (t->bitmap_id != bitmap_id)){

			// lock the bitmap in the proper format
			bmp = bm_lock(bitmap_id, 16, BMP_TEX_XPARENT);
			bm_unlock(bitmap_id);

			// now lets do something for each texture

			for(idx=0; idx<bmp->sections.num_x; idx++){
				for(s_idx=0; s_idx<bmp->sections.num_y; s_idx++){
					// hmm. i'd rather we didn't have to do it this way...
					if(!opengl_create_texture_sectioned(bitmap_id, bitmap_type, t->data_sections[idx][s_idx], idx, s_idx, fail_on_full)){
						ret_val = 0;
					}

					// not used this frame
					t->data_sections[idx][s_idx]->used_this_frame = 0;
				}
			}

			// zero out pretty much everything in the parent struct since he's just the root
			t->bitmap_id = bitmap_id;
			t->texture_handle = 0;
			t->time_created = t->data_sections[sx][sy]->time_created;
			t->used_this_frame = 0;
			/*
			t->vram_texture = NULL;
			t->vram_texture_surface = NULL
			*/
		}

		// argh. we failed to upload. free anything we can
		if(!ret_val){
			opengl_free_texture(t);
		}
		// swap in the texture we want
		else {
			t = t->data_sections[sx][sy];
		}
	}

	// all other "normal" textures
	else if((bitmap_id < 0) || (bitmap_id != t->bitmap_id)){
		ret_val = opengl_create_texture( bitmap_id, bitmap_type, t, fail_on_full );
	}

	// everything went ok
	if(ret_val && (t->texture_handle) && !vram_full){
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;

		
		glBindTexture (GL_TEXTURE_2D, t->texture_handle );

		GL_last_bitmap_id = t->bitmap_id;
		GL_last_bitmap_type = bitmap_type;
		GL_last_section_x = sx;
		GL_last_section_y = sy;
		t->used_this_frame++;
	}
	// gah
	else {
		glBindTexture (GL_TEXTURE_2D, 0);	// test - DDOI
		return 0;
	}

	return 1;
}
//extern int bm_get_cache_slot( int bitmap_id, int separate_ani_frames );
int gr_opengl_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0)
{
	int r1=0,r2=1,r3=1;

	if (bitmap_id < 0)
	{
		GL_last_bitmap_id = -1;
		return 0;
	}

	//make sure textuing is on
	opengl_switch_arb0(1);

	if (GLOWMAP>-1)
	{
		opengl_switch_arb1(1);
		
		r1=gr_opengl_tcache_set_internal(bitmap_id, bitmap_type, u_scale, v_scale, fail_on_full, sx, sy, force, 0);
		r2=gr_opengl_tcache_set_internal(GLOWMAP, bitmap_type, u_scale, v_scale, fail_on_full, sx, sy, force, 1);
	
		//set the glowmap stuff
		glActiveTextureARB(GL_TEXTURE1_ARB);
		gr_opengl_set_additive_tex_env();

		if ((Interp_multitex_cloakmap>0) && (max_multitex > 2))
		{
			opengl_switch_arb2(1);
			glActiveTextureARB(GL_TEXTURE2_ARB);
			gr_opengl_set_additive_tex_env();
			r3=gr_opengl_tcache_set_internal(CLOAKMAP, bitmap_type, u_scale, v_scale, fail_on_full, sx, sy, force, 2);
		}
		else 
		{
			opengl_switch_arb2(0);
			r3=1;
		}
	}
	else
	{
		r1=gr_opengl_tcache_set_internal(bitmap_id, bitmap_type, u_scale, v_scale, fail_on_full, sx, sy, force, 0);
		if (Interp_multitex_cloakmap>0)
		{	
			opengl_switch_arb1(1);
			ogl_maybe_pop_arb1=1;
			gr_push_texture_matrix(1);
			
			glActiveTextureARB(GL_TEXTURE1_ARB);
			gr_opengl_set_additive_tex_env();

			if (max_multitex == 2)
			{
				gr_translate_texture_matrix(1,&tex_shift);
			}
			else
			{
				GLfloat arb2_matrix[16];
				glActiveTextureARB(GL_TEXTURE2_ARB);
				glGetFloatv(GL_TEXTURE_MATRIX, arb2_matrix);
				glActiveTextureARB(GL_TEXTURE1_ARB);
				glMatrixMode(GL_TEXTURE);
				glLoadMatrixf(arb2_matrix);
				glMatrixMode(GL_MODELVIEW);
				opengl_switch_arb2(0);
			}
			r2=gr_opengl_tcache_set_internal(CLOAKMAP, bitmap_type, u_scale, v_scale, fail_on_full, sx, sy, force, 1);

		}
		else 
		{
			r2=1;
			opengl_switch_arb1(0);
			opengl_switch_arb2(0);
		}
		r3=1;
	}

	return ((r1) && (r2) && (r3));

}

void gr_opengl_set_clear_color(int r, int g, int b)
{
	gr_init_color (&gr_screen.current_clear_color, r, g, b);
}

void gr_opengl_flash(int r, int g, int b)
{
	CAP(r,0,255);
	CAP(g,0,255);
	CAP(b,0,255);
	
	if ( r || g || b ) {
		gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
		
		float x1, x2, y1, y2;
		x1 = i2fl(gr_screen.clip_left+gr_screen.offset_x);
		y1 = i2fl(gr_screen.clip_top+gr_screen.offset_y);
		x2 = i2fl(gr_screen.clip_right+gr_screen.offset_x);
		y2 = i2fl(gr_screen.clip_bottom+gr_screen.offset_y);
		
		glColor4ub((ubyte)r, (ubyte)g, (ubyte)b, 255);
		glBegin (GL_QUADS);
		  glVertex3f (x1, y2, -0.99f);

		  glVertex3f (x2, y2, -0.99f);

		  glVertex3f (x2, y1, -0.99f);

		  glVertex3f (x1, y1, -0.99f);
		glEnd ();	  
	}
}

int gr_opengl_zbuffer_get()
{
	if ( !gr_global_zbuffering )    {
		return GR_ZBUFF_NONE;
	}
	return gr_zbuffering_mode;
}

int gr_opengl_zbuffer_set(int mode)
{
	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if (gr_zbuffering_mode == GR_ZBUFF_NONE )      {
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	return tmp;
}

void gr_opengl_zbuffer_clear(int mode)
{
	if (mode) {
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;
		
		gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
		glClear ( GL_DEPTH_BUFFER_BIT );
	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
	}
}

void gr_opengl_set_gamma(float gamma)
{
	Gr_gamma = gamma;
	Gr_gamma_int = int (Gr_gamma*10);

	// Create the Gamma lookup table
	int i;
	for (i=0;i<256; i++) {
		int v = fl2i(pow(i2fl(i)/255.0f, 1.0f/Gr_gamma)*255.0f);
		if ( v > 255 ) {
			v = 255;
		} else if ( v < 0 )     {
			v = 0;
		}
		Gr_gamma_lookup[i] = v;
	}

	// Flush any existing textures
	opengl_tcache_flush();
}

void gr_opengl_fade_in(int instantaneous)
{
	// Empty - DDOI
}

void gr_opengl_fade_out(int instantaneous)
{
	// Empty - DDOI
}

void gr_opengl_get_region(int front, int w, int h, ubyte *data)
{

//	if (front) {
//		glReadBuffer(GL_FRONT);
//	} else {
		glReadBuffer(GL_BACK);
//	}

	gr_opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);
	
	if (gr_screen.bits_per_pixel == 16) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data);
	} else if (gr_screen.bits_per_pixel == 32) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}


}


#define MAX_SAVE_SIZE (32*32)
static ubyte Gr_opengl_mouse_saved_data[MAX_SAVE_SIZE*2];

#define CLAMP(x,r1,r2) do { if ( (x) < (r1) ) (x) = (r1); else if ((x) > (r2)) (x) = (r2); } while(0)

void gr_opengl_save_mouse_area(int x, int y, int w, int h)
{
	Gr_opengl_mouse_saved_x1 = x;
	Gr_opengl_mouse_saved_y1 = y;
	Gr_opengl_mouse_saved_x2 = x+w-1;
	Gr_opengl_mouse_saved_y2 = y+h-1;
        
	CLAMP(Gr_opengl_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_opengl_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_opengl_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(Gr_opengl_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );
        
	Gr_opengl_mouse_saved_w = Gr_opengl_mouse_saved_x2 - Gr_opengl_mouse_saved_x1 + 1;
	Gr_opengl_mouse_saved_h = Gr_opengl_mouse_saved_y2 - Gr_opengl_mouse_saved_y1 + 1;

	if ( Gr_opengl_mouse_saved_w < 1 ) return;
	if ( Gr_opengl_mouse_saved_h < 1 ) return;
        
	Assert( (Gr_opengl_mouse_saved_w*Gr_opengl_mouse_saved_h) <= MAX_SAVE_SIZE );

	gr_opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);
        
	glReadBuffer(GL_BACK);
	glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Gr_opengl_mouse_saved_data);
        
	Gr_opengl_mouse_saved = 1;
}

ubyte *opengl_screen=NULL;
GLenum screen_tex_handle;
int scr_bm;
float maxu, maxv;

int gr_opengl_save_screen()
{
	GLenum fmt=0;
	gr_reset_clip();

	if ( opengl_screen )  {
		mprintf(( "Screen already saved!\n" ));
		return -1;
	}

	fmt=GL_UNSIGNED_SHORT_1_5_5_5_REV;
	opengl_screen = (ubyte*)malloc( gr_screen.max_w * gr_screen.max_h * gr_screen.bytes_per_pixel);

	ubyte *opengl_screen_tmp = (ubyte*)malloc( gr_screen.max_w * gr_screen.max_h * gr_screen.bytes_per_pixel );
	if (!opengl_screen_tmp) 
	{
		mprintf(( "Couldn't get memory for temporary saved screen!\n" ));
		return -1;
	}

	if (!opengl_screen) 
	{
		mprintf(( "Couldn't get memory for saved screen!\n" ));
		return -1;
	}
	

	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGRA, fmt, opengl_screen_tmp);
        
	ubyte *sptr, *dptr;
        
	sptr = (ubyte *)&opengl_screen_tmp[gr_screen.max_w*gr_screen.max_h*2];
	dptr = (ubyte *)opengl_screen;
	for (int j = 0; j < gr_screen.max_h; j++)
	{
		sptr -= gr_screen.max_w*2;
		memcpy(dptr, sptr, gr_screen.max_w*2);
		dptr += gr_screen.max_w*2;
	}
        
	free(opengl_screen_tmp);
        
	if (Gr_opengl_mouse_saved)
	{
		sptr = (ubyte *)Gr_opengl_mouse_saved_data;
		dptr = (ubyte *)&opengl_screen[2*(Gr_opengl_mouse_saved_x1+(Gr_opengl_mouse_saved_y2)*gr_screen.max_w)];
		for (int i = 0; i < Gr_opengl_mouse_saved_h; i++)
		{
			memcpy(dptr, sptr, Gr_opengl_mouse_saved_w*2);
			sptr += 32*2;
			dptr -= gr_screen.max_w*2;
		}
	}

	scr_bm=bm_create(16, gr_screen.max_w, gr_screen.max_h, opengl_screen, 0);

	return 1;
}

void gr_opengl_restore_screen(int id)
{
	gr_reset_clip();
	
	if ( !opengl_screen ) {
		gr_clear();
		return;
	}

	gr_set_bitmap(scr_bm);
	gr_bitmap(0,0);


}

void gr_opengl_free_screen(int id)
{
	if (!opengl_screen)
		return;

	free(opengl_screen);
	opengl_screen=NULL;

	bm_release(scr_bm);
}

void gr_opengl_dump_frame_start(int first_frame, int frames_between_dumps)
{
	STUB_FUNCTION;
}

void gr_opengl_dump_frame_stop()
{
	STUB_FUNCTION;
}

void gr_opengl_dump_frame()
{
	STUB_FUNCTION;
}

uint gr_opengl_lock()
{
	STUB_FUNCTION;
	
	return 1;
}
        
void gr_opengl_unlock()
{
}

void opengl_zbias(int bias)
{
	if (bias) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0, -i2fl(bias));
	} else {
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}
       
void gr_opengl_bitmap_internal(int x,int y,int w,int h,int sx,int sy)
{

	//int i,j,k;
	unsigned int tex=0;			//gl texture number

	bitmap * bmp;
	int size;


	int htemp=(int)pow(2,ceil(log10(h)/log10(2)));
	int wtemp=(int)pow(2,ceil(log10(w)/log10(2)));

	//gr_opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);

	glGenTextures(1, &tex);

	Assert(tex !=0);
  	//Assert(opengl_screen != NULL);
	//Assert(opengl_bmp_buffer != NULL);

	// mharris mod - not sure if this is right...
	bmp = bm_lock( gr_screen.current_bitmap, 16, 0 );
	size=w*h*4;

	/*mprintf(("gr_opengl_bitmap_ex_internal: at (%3d,%3d) size (%3d,%3d) name %s -- temp (%d,%d)\n", 
  				x, y, w, h, 
 				bm_get_filename(gr_screen.current_bitmap),wtemp,htemp));*/

	const ushort * sptr = (const ushort*)bmp->data;

	glColor3ub(255,255,255);

	glBindTexture(GL_TEXTURE_2D,tex);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, wtemp, htemp, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
	glTexSubImage2D(GL_TEXTURE_2D, 0,0,0,w,h, GL_BGRA,GL_UNSIGNED_SHORT_1_5_5_5_REV, sptr);
	
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2i(x,y);

		glTexCoord2f(0,i2fl(h)/i2fl(htemp));
		glVertex2i(x,y+h);

		glTexCoord2f(i2fl(w)/i2fl(wtemp),i2fl(h)/i2fl(htemp));
		glVertex2i(x+w,y+h);

		glTexCoord2f(i2fl(w)/i2fl(wtemp),0);
		glVertex2i(x+w,y);
	glEnd();

	bm_unlock(gr_screen.current_bitmap);

	glDeleteTextures(1, &tex);

//	// set the raster pos
/*	int gl_y_origin = y+h;

	glRasterPos2i(gr_screen.offset_x + x, gl_y_origin);

	// put the bitmap into the GL framebuffer
	// BAD BAD BAD - change this asap
	glDrawPixels(w, h,					// width, height
					 GL_RGBA,			// format
					 GL_UNSIGNED_BYTE,	// type
					 data);
	*/
}


//these are penguins bitmap functions
void gr_opengl_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	gr_opengl_bitmap_internal(x,y,w,h,sx,sy);

}

void gr_opengl_bitmap(int x, int y)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );
	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
	if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
	if ( dx1 < gr_screen.clip_left ) { sx = gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
	if ( dy1 < gr_screen.clip_top ) { sy = gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
	if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
	if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	gr_bitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_opengl_push_texture_matrix(int unit)
{
	if (unit > max_multitex) return;
	GLint current_matrix;
	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	glActiveTextureARB(GL_TEXTURE0_ARB+unit);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glMatrixMode(current_matrix);
}

void gr_opengl_pop_texture_matrix(int unit)
{
	if (unit > max_multitex) return;
	GLint current_matrix;
	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	glActiveTextureARB(GL_TEXTURE0_ARB+unit);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(current_matrix);
}

void gr_opengl_translate_texture_matrix(int unit, vector *shift)
{
	if (unit > max_multitex){ tex_shift=*shift; return;}
	GLint current_matrix;
	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	glActiveTextureARB(GL_TEXTURE0_ARB+unit);
	glMatrixMode(GL_TEXTURE);
	glTranslated(shift->xyz.x, shift->xyz.y, shift->xyz.z);	
	glMatrixMode(current_matrix);
	tex_shift=vmd_zero_vector;
}


struct opengl_vertex_buffer
{
	int n_poly;
	vector *vertex_array;
	uv_pair *texcoord_array;
	vector *normal_array;

	int used;

	uint vbo_vert;
	uint vbo_norm;
	uint vbo_tex;

};

#define MAX_SUBOBJECTS 64

#ifdef INF_BUILD
#define MAX_BUFFERS_PER_SUBMODEL 24
#else
#define MAX_BUFFERS_PER_SUBMODEL 16
#endif

#define MAX_BUFFERS MAX_POLYGON_MODELS*MAX_SUBOBJECTS*MAX_BUFFERS_PER_SUBMODEL

opengl_vertex_buffer vertex_buffers[MAX_BUFFERS];

//zeros everything out
void opengl_init_vertex_buffers()
{
	memset(vertex_buffers,0,sizeof(opengl_vertex_buffer)*MAX_BUFFERS);
}

int opengl_find_first_free_buffer()
{
	for (int i=0; i < MAX_BUFFERS; i++)
	{
		if (!vertex_buffers[i].used)
			return i;
	}
	
	return -1;
}

int opengl_check_for_errors()
{
#ifdef _DEBUG
	int error=0;
	if ((error=glGetError()) != GL_NO_ERROR)
	{
		mprintf(("!!ERROR!!: %s\n", gluErrorString(error)));
		return 1;
	}
#endif
	return 0;
}

int opengl_mod_depth()
{
	int mv;
	glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &mv);
	return mv;
}

uint opengl_create_vbo(uint size, void** data)
{
	if (!data)
		return 0;

	if (!*data)
		return 0;

	if (size == 0)
		return 0;


	// Kazan: A) This makes that if (buffer_name) work correctly (false = 0, true = anything not 0)
	//				if glGenBuffersARB() doesn't initialized it for some reason
	//        B) It shuts up MSVC about may be used without been initalized
	uint buffer_name=0;

	glGenBuffersARB(1, &buffer_name);
	
	//make sure we have one
	if (buffer_name)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer_name);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, *data, GL_STATIC_DRAW_ARB );
				
		//just in case
		if (opengl_check_for_errors())
		{
			return 0;
		}
		else
		{
			free(*data);
			*data = NULL;
		//	mprintf(("VBO Created: %d\n", buffer_name));
		}
	}

	return buffer_name;
}

int gr_opengl_make_buffer(poly_list *list)
{
	int buffer_num=opengl_find_first_free_buffer();

	//we have a valid buffer
	if (buffer_num > -1)
	{
		opengl_vertex_buffer *vbp=&vertex_buffers[buffer_num];

		vbp->used=1;

		vbp->n_poly=list->n_poly;

		vbp->texcoord_array=(uv_pair*)malloc(list->n_poly * 3 * sizeof(uv_pair));	
		memset(vbp->texcoord_array,0,list->n_poly*sizeof(uv_pair));

		vbp->normal_array=(vector*)malloc(list->n_poly * 3 * sizeof(vector));
		memset(vbp->normal_array,0,list->n_poly*sizeof(vector));

		vbp->vertex_array=(vector*)malloc(list->n_poly * 3 * sizeof(vector));
		memset(vbp->vertex_array,0,list->n_poly*sizeof(vector));

		vector *n=vbp->normal_array;
		vector *v=vbp->vertex_array;
		uv_pair *t=vbp->texcoord_array;
	
		vertex *vl;

		memcpy(n,list->norm,list->n_poly*sizeof(vector)*3);
				

		for (int i=0; i < list->n_poly*3; i++)
		{
				vl=&list->vert[i];
				v->xyz.x=vl->x; 
				v->xyz.y=vl->y;
				v->xyz.z=vl->z;
				v++;
				
				t->u=vl->u;
				t->v=vl->v;
				t++;

		}

		//maybe load it into a vertex buffer object
		if (VBO_ENABLED)
		{
			vbp->vbo_vert=opengl_create_vbo(vbp->n_poly*9*sizeof(float),(void**)&vbp->vertex_array);
			vbp->vbo_norm=opengl_create_vbo(vbp->n_poly*9*sizeof(float),(void**)&vbp->normal_array);
			vbp->vbo_tex=opengl_create_vbo(vbp->n_poly*6*sizeof(float),(void**)&vbp->texcoord_array);
		}

	}
	

	return buffer_num;
}
	
void gr_opengl_destroy_buffer(int idx)
{
	opengl_vertex_buffer *vbp=&vertex_buffers[idx];
	if (vbp->normal_array)		free(vbp->normal_array);
	if (vbp->texcoord_array)	free(vbp->texcoord_array);
	if (vbp->vertex_array)		free(vbp->vertex_array);

	if (vbp->vbo_norm)			glDeleteBuffersARB(1,&vbp->vbo_norm);
	if (vbp->vbo_vert)			glDeleteBuffersARB(1,&vbp->vbo_vert);
	if (vbp->vbo_tex)			glDeleteBuffersARB(1,&vbp->vbo_tex);

	memset(vbp,0,sizeof(opengl_vertex_buffer));
}

//#define DRAW_DEBUG_LINES
extern float Model_Interp_scale_x,Model_Interp_scale_y,Model_Interp_scale_z;
void gr_opengl_render_buffer(int idx)
{
	TIMERBAR_PUSH(2);
	float u_scale,v_scale;

	if (glIsEnabled(GL_CULL_FACE))	glFrontFace(GL_CW);
	
//	glColor3ub(191,191,191);
	
	opengl_vertex_buffer *vbp=&vertex_buffers[idx];

	glEnableClientState(GL_VERTEX_ARRAY);
	if (vbp->vbo_vert)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_vert);
		glVertexPointer(3,GL_FLOAT,0, (void*)NULL);
	}
	else
	{
		glVertexPointer(3,GL_FLOAT,0,vbp->vertex_array);
	}

	glEnableClientState(GL_NORMAL_ARRAY);
	if (vbp->vbo_norm)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_norm);
		glNormalPointer(GL_FLOAT,0, (void*)NULL);
	}
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glNormalPointer(GL_FLOAT,0,vbp->normal_array);
	}

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (vbp->vbo_tex)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_tex);
		glTexCoordPointer(2,GL_FLOAT,0,(void*)NULL);
	}
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glTexCoordPointer(2,GL_FLOAT,0,vbp->texcoord_array);
	}

	if (GLOWMAP > -1)
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo_tex)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_tex);
			glTexCoordPointer(2,GL_FLOAT,0,(void*)NULL);
		}
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glTexCoordPointer(2,GL_FLOAT,0,vbp->texcoord_array);
		}
	}

	if ((Interp_multitex_cloakmap>0) && (max_multitex > 2))
	{
		glClientActiveTextureARB(GL_TEXTURE2_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo_tex)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_tex);
			glTexCoordPointer(2,GL_FLOAT,0,(void*)NULL);
		}
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glTexCoordPointer(2,GL_FLOAT,0,vbp->texcoord_array);
		}
	}


	int r,g,b,a,tmap_type;

	opengl_setup_render_states(r,g,b,a,tmap_type,TMAP_FLAG_TEXTURED,0);

	if (gr_screen.current_bitmap==CLOAKMAP)
	{
		glBlendFunc(GL_ONE,GL_ONE);
		r=g=b=Interp_cloakmap_alpha;
		a=255;
	}

	gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0);
	
	glLockArraysEXT(0,vbp->n_poly*3);
	
	pre_render_init_lights();
	change_active_lights(0);

	glDrawArrays(GL_TRIANGLES,0,vbp->n_poly*3);

	if((lighting_is_enabled)&&((n_active_lights-1)/max_gl_lights > 0)) {
		gr_opengl_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
		opengl_switch_arb1(0);
		opengl_switch_arb2(0);
		for(int i=1; i< (n_active_lights-1)/max_gl_lights; i++)
		{
			change_active_lights(i);
			glDrawArrays(GL_TRIANGLES,0,vbp->n_poly*3); 
		}
	}

	glUnlockArraysEXT();


	if (ogl_maybe_pop_arb1)
	{
		gr_pop_texture_matrix(1);
		ogl_maybe_pop_arb1=0;
	}

	TIMERBAR_POP();

	if (VBO_ENABLED)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}


#if defined(DRAW_DEBUG_LINES) && defined(_DEBUG)
	glBegin(GL_LINES);
		glColor3ub(255,0,0);
		glVertex3d(0,0,0);
		glVertex3d(20,0,0);

		glColor3ub(0,255,0);
		glVertex3d(0,0,0);
		glVertex3d(0,20,0);

		glColor3ub(0,0,255);
		glVertex3d(0,0,0);
		glVertex3d(0,0,20);
	glEnd();
#endif

	
}

void gr_opengl_start_instance_matrix(vector *offset, matrix* rotation)
{
	if (!offset)
		offset = &vmd_zero_vector;
	if (!rotation)
		rotation = &vmd_identity_matrix;	

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	vector axis;
	float ang;
	vm_matrix_to_rot_axis_and_angle(rotation,&ang,&axis);
	glTranslatef(offset->xyz.x,offset->xyz.y,offset->xyz.z);
	glRotatef(fl_degrees(ang),axis.xyz.x,axis.xyz.y,axis.xyz.z);
	depth++;
}

void gr_opengl_start_instance_angles(vector *pos, angles* rotation)
{
	matrix m;
	vm_angles_2_matrix(&m,rotation);
	gr_opengl_start_instance_matrix(pos,&m);
}

void gr_opengl_end_instance_matrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	depth--;
}

//the projection matrix; fov, aspect ratio, near, far
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(fl_degrees(fov),aspect,z_near,z_far);
	glMatrixMode(GL_MODELVIEW);
}

void gr_opengl_end_projection_matrix()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void gr_opengl_set_view_matrix(vector *pos, matrix* orient)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
	
	vector fwd;
	vector *uvec=&orient->vec.uvec;

	vm_vec_add(&fwd, pos, &orient->vec.fvec);

	gluLookAt(pos->xyz.x,pos->xyz.y,-pos->xyz.z,
	fwd.xyz.x,fwd.xyz.y,-fwd.xyz.z,
	uvec->xyz.x, uvec->xyz.y,-uvec->xyz.z);

	glScalef(1,1,-1);
	
	depth=2;

	glViewport(gr_screen.offset_x,gr_screen.max_h-gr_screen.offset_y-gr_screen.clip_height,gr_screen.clip_width,gr_screen.clip_height);
}

void gr_opengl_end_view_matrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glLoadIdentity();
	depth=1;

	glViewport(0,0,gr_screen.max_w, gr_screen.max_h);
	
}

void gr_opengl_push_scale_matrix(vector *scale_factor)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	depth++;
	glScalef(scale_factor->xyz.x,scale_factor->xyz.y,scale_factor->xyz.z);
}

void gr_opengl_pop_scale_matrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	depth--;
}
/*
int gr_opengl_make_light(light_data* light, int idx, int priority)
{
	//stubb
	return -1;
}

void gr_opengl_modify_light(light_data* light, int idx, int priority)
{
	//stubb
}

void gr_opengl_destroy_light(int idx)
{
	//stubb
}

void gr_opengl_set_light(light_data *light)
{
	//stubb
}

void gr_opengl_set_lighting(bool set, bool state)
{

}

void gr_opengl_reset_lighting()
{

}
*/
void gr_opengl_end_clip_plane()
{
	glDisable(GL_CLIP_PLANE0);
}

void gr_opengl_start_clip_plane()
{	
	double clip_equation[4];
	vector n;
	vector p;

	n=G3_user_clip_normal;	
	p=G3_user_clip_point;

	clip_equation[0]=n.xyz.x;
	clip_equation[1]=n.xyz.y;
	clip_equation[2]=n.xyz.z;
	clip_equation[3]=-vm_vec_dot(&p, &n);
	glClipPlane(GL_CLIP_PLANE0, clip_equation);
	glEnable(GL_CLIP_PLANE0);
}

void opengl_render_timer_bar(int colour, float x, float y, float w, float h)
{
	static float pre_set_colours[MAX_NUM_TIMERBARS][3] = 
	{
		1.0,0,
		0,1.0,
		0,0,1,

		0.2f,1.0f,0.8f, 
		1.0f,0.0f,8.0f, 
		1.0f,0.0f,1.0f,
		1.0f,0.2f,0.2f,
		1.0f,1.0f,1.0f
	};

	static float max_fw = (float) gr_screen.max_w; 
	static float max_fh = (float) gr_screen.max_h; 

	x *= max_fw;
	y *= max_fh;
	w *= max_fw;
	h *= max_fh;

	y += 5;

	gr_opengl_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);

	glColor3fv(pre_set_colours[colour]);

	glBegin(GL_QUADS);
		glVertex2f(x,y);
		glVertex2f(x,y+h);
		glVertex2f(x+w,y+h);
		glVertex2f(x+w,y);
	glEnd();
}

extern char *Osreg_title;
void gr_opengl_init(int reinit)
{
	char *extlist;
	char *curext;
	char *ver;
	char curver[3];
	int bpp = gr_screen.bits_per_pixel;

	if(!Cmdline_nohtl) {
		opengl_init_vertex_buffers();
	}

	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));

	switch( bpp )	{
	case 16:
	/*
		Gr_red=Gr_t_red;
		Gr_green=Gr_t_green;
		Gr_blue=Gr_t_blue;
		Gr_alpha=Gr_t_alpha;
	*/
		Gr_red.bits = 5;
		Gr_red.shift = 11;
		Gr_red.scale = 8;
		Gr_red.mask = 0xF800;

		Gr_green.bits = 6;
		Gr_green.shift = 5;
		Gr_green.scale = 4;
		Gr_green.mask = 0x7E0;

		Gr_blue.bits = 5;
		Gr_blue.shift = 0;
		Gr_blue.scale = 8;
		Gr_blue.mask = 0x1F;		

		break;

	case 32:
		Gr_red.bits = 8;
		Gr_red.shift = 16;
		Gr_red.scale = 1;
		Gr_red.mask = 0xff0000;

		Gr_green.bits = 8;
		Gr_green.shift = 8;
		Gr_green.scale = 1;
		Gr_green.mask = 0x00ff00;

		Gr_blue.bits = 8;
		Gr_blue.shift = 0;
		Gr_blue.scale = 1;
		Gr_blue.mask = 0x0000ff;

		Gr_alpha.bits=8;
		Gr_alpha.shift=24;
		Gr_alpha.mask=0xff000000;
		Gr_alpha.scale=1;


		break;

	default:
		mprintf(("Illegal BPP passed to gr_opengl_init() -- %d",bpp));
		Int3();	// Illegal bpp
	}

	Gr_t_red.bits=5;
	Gr_t_red.mask = 0x7c00;
	Gr_t_red.shift = 10;
	Gr_t_red.scale = 8;

	Gr_t_green.bits=5;
	Gr_t_green.mask = 0x03e0;
	Gr_t_green.shift = 5;
	Gr_t_green.scale = 8;
	
	Gr_t_blue.bits=5;
	Gr_t_blue.mask = 0x001f;
	Gr_t_blue.shift = 0;
	Gr_t_blue.scale = 8;

	Gr_t_alpha.bits=1;
	Gr_t_alpha.mask=0x8000;
	Gr_t_alpha.scale=255;
	Gr_t_alpha.shift=15;

	/*
Gr_ta_red: bits=0, mask=f00, scale=17, shift=8
Gr_ta_green: bits=0, mask=f00, scale=17, shift=4
Gr_ta_blue: bits=0, mask=f00, scale=17, shift=0
Gr_ta_alpha: bits=0, mask=f000, scale=17, shift=c
*/
	// DDOI - set these so no one else does!
	Gr_ta_red.bits=4;
	Gr_ta_red.mask = 0x0f00;
	Gr_ta_red.shift = 8;
	Gr_ta_red.scale = 17;

	Gr_ta_green.bits=4;
	Gr_ta_green.mask = 0x00f0;
	Gr_ta_green.shift = 4;
	Gr_ta_green.scale = 17;
	
	Gr_ta_blue.bits=4;
	Gr_ta_blue.mask = 0x000f;
	Gr_ta_blue.shift = 0;
	Gr_ta_blue.scale = 17;

	Gr_ta_alpha.bits=4;
	Gr_ta_alpha.mask = 0xf000;
	Gr_ta_alpha.shift = 12;
	Gr_ta_alpha.scale = 17;

	if ( OGL_inited )	{
		gr_opengl_cleanup();
		OGL_inited = 0;
	}

	mprintf(( "Initializing opengl graphics device...\nRes:%dx%dx%d\n",gr_screen.max_w,gr_screen.max_h,gr_screen.bits_per_pixel ));

	pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion=1;
	pfd.cColorBits=(ubyte)bpp;
	pfd.dwFlags=PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cDepthBits=24;
	pfd.iPixelType=PFD_TYPE_RGBA;
	pfd.cRedBits=(ubyte)Gr_red.bits;
	pfd.cRedShift=(ubyte)Gr_red.shift;
	pfd.cBlueBits=(ubyte)Gr_blue.bits;
	pfd.cBlueShift=(ubyte)Gr_blue.shift;
	pfd.cGreenBits=(ubyte)Gr_green.bits;
	pfd.cGreenShift=(ubyte)Gr_green.shift;


	int PixelFormat;

	HWND wnd=(HWND)os_get_window();

#ifndef FRED
	Assert(wnd);
	
	dev_context=GetDC(wnd);
	if (!dev_context)
	{
		MessageBox(wnd, "Unable to get Device context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);		
	}

	PixelFormat=ChoosePixelFormat(dev_context, &pfd);
	if (!PixelFormat)
	{
		MessageBox(wnd, "Unable to choose pixel format for OpenGL W32!","error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	if (!SetPixelFormat(dev_context, PixelFormat, &pfd))
	{
		MessageBox(wnd, "Unable to set pixel format for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	rend_context=wglCreateContext(dev_context);
	if (!rend_context)
	{
		MessageBox(wnd, "Unable to create rendering context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);
	}
	
	if (!wglMakeCurrent(dev_context, rend_context))
	{
		MessageBox(wnd, "Unable to make current thread for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	ver=(char*)glGetString(GL_VERSION);
	OGL_inited = 1;
	if (!reinit)
	{
		mprintf(("OPENGL INITED!\n"));
		mprintf(("\n"));
		mprintf(( "Vendor     : %s\n", glGetString( GL_VENDOR ) ));
		mprintf(( "Renderer   : %s\n", glGetString( GL_RENDERER ) ));
		mprintf(( "Version    : %s\n", ver ));
		mprintf(( "Extensions : \n" ));

		//print out extensions
		OGL_extensions=(const char*)glGetString(GL_EXTENSIONS);

		extlist=(char*)malloc(strlen(OGL_extensions));
		memcpy(extlist, OGL_extensions, strlen(OGL_extensions));
		memcpy(curver, ver,3);

		float version_float=(float)atof(curver);
		if (version_float < REQUIRED_GL_VERSION)
		{
			Error(LOCATION,"Current GL Version of %f is less than required version of %f\nSwitch video modes or update drivers", version_float, REQUIRED_GL_VERSION);
		}
	
		curext=strtok(extlist, " ");
		while (curext)
		{
			mprintf(( "%s\n", curext ));
			curext=strtok(NULL, " ");
		}
		free(extlist);
	}

#endif

	glGetIntegerv(GL_MAX_LIGHTS, &max_gl_lights); //Get the max number of lights supported
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

#ifndef FRED

	if (!Cmdline_window)
	{
		opengl_go_fullscreen(wnd);
	}
#endif

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, gr_screen.max_w, gr_screen.max_h,0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DITHER);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_FOG_HINT, GL_NICEST);
		
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	
	glEnable(GL_TEXTURE_2D);
	
	glDepthRange(0.0, 1.0);
	
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glFlush();
	
	Bm_pixel_format = BM_PIXEL_FORMAT_ARGB;
		

	Gr_bitmap_poly = 1;
	OGL_fogmode=1;

	glEnable(GL_COLOR_SUM_EXT);
	
	
#ifndef FRED

	if (!reinit)
		//start extension
		opengl_get_extensions();

#endif

	if (!reinit)
		opengl_tcache_init (1);


	gr_opengl_clear();

	Gr_current_red = &Gr_red;
	Gr_current_blue = &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;
                                
	gr_screen.gf_flip = gr_opengl_flip;
	gr_screen.gf_flip_window = gr_opengl_flip_window;
	gr_screen.gf_set_clip = gr_opengl_set_clip;
	gr_screen.gf_reset_clip = gr_opengl_reset_clip;
	gr_screen.gf_set_font = grx_set_font;
	
	gr_screen.gf_set_color = gr_opengl_set_color;
	gr_screen.gf_set_bitmap = gr_opengl_set_bitmap;
	gr_screen.gf_create_shader = gr_opengl_create_shader;
	gr_screen.gf_set_shader = gr_opengl_set_shader;
	gr_screen.gf_clear = gr_opengl_clear;
//	gr_screen.gf_bitmap = gr_opengl_bitmap;
//	gr_screen.gf_bitmap_ex = gr_opengl_bitmap_ex;
	gr_screen.gf_aabitmap = gr_opengl_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_opengl_aabitmap_ex;
	
	gr_screen.gf_rect = gr_opengl_rect;
	gr_screen.gf_shade = gr_opengl_shade;
	gr_screen.gf_string = gr_opengl_string;
	gr_screen.gf_circle = gr_opengl_circle;

	gr_screen.gf_line = gr_opengl_line;
	gr_screen.gf_aaline = gr_opengl_aaline;
	gr_screen.gf_pixel = gr_opengl_pixel;
	gr_screen.gf_scaler = gr_opengl_scaler;
	gr_screen.gf_tmapper = gr_opengl_tmapper;

	gr_screen.gf_gradient = gr_opengl_gradient;

	gr_screen.gf_set_palette = gr_opengl_set_palette;
	gr_screen.gf_get_color = gr_opengl_get_color;
	gr_screen.gf_init_color = gr_opengl_init_color;
	gr_screen.gf_init_alphacolor = gr_opengl_init_alphacolor;
	gr_screen.gf_set_color_fast = gr_opengl_set_color_fast;
	gr_screen.gf_print_screen = gr_opengl_print_screen;

	gr_screen.gf_fade_in = gr_opengl_fade_in;
	gr_screen.gf_fade_out = gr_opengl_fade_out;
	gr_screen.gf_flash = gr_opengl_flash;
	
	gr_screen.gf_zbuffer_get = gr_opengl_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_opengl_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_opengl_zbuffer_clear;
	
	gr_screen.gf_save_screen = gr_opengl_save_screen;
	gr_screen.gf_restore_screen = gr_opengl_restore_screen;
	gr_screen.gf_free_screen = gr_opengl_free_screen;
	
	gr_screen.gf_dump_frame_start = gr_opengl_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_opengl_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_opengl_dump_frame;
	
	gr_screen.gf_set_gamma = gr_opengl_set_gamma;
	
	gr_screen.gf_lock = gr_opengl_lock;
	gr_screen.gf_unlock = gr_opengl_unlock;
	
	gr_screen.gf_fog_set = gr_opengl_fog_set;	

	// UnknownPlayer : Don't recognize this - MAY NEED DEBUGGING
	gr_screen.gf_get_region = gr_opengl_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_get_next_handle         = bm_gfx_get_next_handle;         
	gr_screen.gf_bm_close                   = bm_gfx_close;                   
	gr_screen.gf_bm_init                    = bm_gfx_init;                    
	gr_screen.gf_bm_get_frame_usage         = bm_gfx_get_frame_usage;         
	gr_screen.gf_bm_create                  = bm_gfx_create;                  
	gr_screen.gf_bm_load                    = bm_gfx_load;                   
	gr_screen.gf_bm_load_duplicate          = bm_gfx_load_duplicate;          
	gr_screen.gf_bm_load_animation          = bm_gfx_load_animation;          
	gr_screen.gf_bm_get_info                = bm_gfx_get_info;                
	gr_screen.gf_bm_lock                    = bm_gfx_lock;                    
	gr_screen.gf_bm_unlock                  = bm_gfx_unlock;                  
	gr_screen.gf_bm_get_palette             = bm_gfx_get_palette;             
	gr_screen.gf_bm_release                 = bm_gfx_release;                 
	gr_screen.gf_bm_unload                  = bm_gfx_unload;                  
	gr_screen.gf_bm_unload_all              = bm_gfx_unload_all;              
	gr_screen.gf_bm_page_in_texture         = bm_gfx_page_in_texture;         
	gr_screen.gf_bm_page_in_start           = bm_gfx_page_in_start;           
	gr_screen.gf_bm_page_in_stop            = bm_gfx_page_in_stop;            
	gr_screen.gf_bm_get_cache_slot          = bm_gfx_get_cache_slot;          
	gr_screen.gf_bm_get_components          = bm_gfx_get_components;          
	gr_screen.gf_bm_get_section_size        = bm_gfx_get_section_size;

	gr_screen.gf_bm_page_in_nondarkening_texture = bm_gfx_page_in_nondarkening_texture; 
	gr_screen.gf_bm_page_in_xparent_texture		 = bm_gfx_page_in_xparent_texture;		 
	gr_screen.gf_bm_page_in_aabitmap			 = bm_gfx_page_in_aabitmap;

//	RunGLTest( 0, NULL, 0, 0, 0, 0.0, 0 );

	gr_screen.gf_get_pixel = gr_opengl_get_pixel;

	gr_screen.gf_set_cull = gr_opengl_set_cull;

	gr_screen.gf_cross_fade = gr_opengl_cross_fade;

	gr_screen.gf_filter_set = gr_opengl_filter_set;

	gr_screen.gf_tcache_set = gr_opengl_tcache_set;

	gr_screen.gf_set_clear_color = gr_opengl_set_clear_color;

	gr_screen.gf_push_texture_matrix = gr_opengl_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix = gr_opengl_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix = gr_opengl_translate_texture_matrix;

	if(!Cmdline_nohtl) {
		gr_screen.gf_make_buffer = gr_opengl_make_buffer;
		gr_screen.gf_destroy_buffer = gr_opengl_destroy_buffer;
		gr_screen.gf_render_buffer = gr_opengl_render_buffer;

		gr_screen.gf_start_instance_matrix = gr_opengl_start_instance_matrix;
		gr_screen.gf_end_instance_matrix = gr_opengl_end_instance_matrix;
		gr_screen.gf_start_angles_instance_matrix = gr_opengl_start_instance_angles;


		gr_screen.gf_make_light = gr_opengl_make_light;
		gr_screen.gf_modify_light = gr_opengl_modify_light;
		gr_screen.gf_destroy_light = gr_opengl_destroy_light;
		gr_screen.gf_set_light = gr_opengl_set_light;
		gr_screen.gf_reset_lighting = gr_opengl_reset_lighting;

		gr_screen.start_clip_plane = gr_opengl_start_clip_plane;
		gr_screen.end_clip_plane = gr_opengl_end_clip_plane;

		gr_screen.gf_lighting = gr_opengl_set_lighting;

		gr_screen.gf_set_proj_matrix=gr_opengl_set_projection_matrix;
		gr_screen.gf_end_proj_matrix=gr_opengl_end_projection_matrix;

		gr_screen.gf_set_view_matrix=gr_opengl_set_view_matrix;
		gr_screen.gf_end_view_matrix=gr_opengl_end_view_matrix;

		gr_screen.gf_push_scale_matrix = gr_opengl_push_scale_matrix;
		gr_screen.gf_pop_scale_matrix = gr_opengl_pop_scale_matrix;

//		glEnable(GL_NORMALIZE);
	}
	else	//use some function stubs
	{
		gr_screen.gf_make_light = gr_opengl_make_light;
		gr_screen.gf_modify_light = gr_opengl_modify_light;
		gr_screen.gf_destroy_light = gr_opengl_destroy_light;
		gr_screen.gf_set_light = gr_opengl_set_light;
		gr_screen.gf_reset_lighting = gr_opengl_reset_lighting;


		gr_screen.gf_lighting = gr_opengl_set_lighting;


	}

	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	//if S3TC compression is found, then "GL_ARB_texture_compression" must be an extension
	Texture_compression_enabled=((GL_Extensions[GL_TEX_COMP_S3TC].enabled) && (GL_Extensions[GL_COMP_TEX].enabled));

	//setup anisotropic filtering if found
	if (GL_Extensions[GL_TEX_FILTER_ANSIO].enabled)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	}


	//allow VBOs to be used
	if ( (!Cmdline_nohtl) && (GL_Extensions[GL_ARB_VBO_BIND_BUFFER].enabled))
		VBO_ENABLED = 1;
	
//	glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
//	glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	arb0_enabled=1;

	//setup the best fog function found
	//start with NV Radial Fog (GeForces only)  -- needs t&l, will have to wait
	//if (GL_Extensions[GL_NV_RADIAL_FOG].enabled)
	//	OGL_fogmode=3;
	/*else*/
	if (GL_Extensions[GL_FOG_COORDF].enabled)
		OGL_fogmode=2;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &max_multitex);

	if (max_multitex > 2)	gr_opengl_tmapper_internal = gr_opengl_tmapper_internal_3multitex;
	else					gr_opengl_tmapper_internal = gr_opengl_tmapper_internal_2multitex;

	if (GL_Extensions[GL_ARB_ENV_COMBINE].enabled)
		gr_opengl_set_tex_src = gr_opengl_set_tex_state_combine_arb;
	else if (GL_Extensions[GL_EXT_ENV_COMBINE].enabled)
		gr_opengl_set_tex_src = gr_opengl_set_tex_state_combine_ext;
	else
		gr_opengl_set_tex_src = gr_opengl_set_tex_state_no_combine;

	glDisable(GL_LIGHTING); //making sure of it

	TIMERBAR_SET_DRAW_FUNC(opengl_render_timer_bar);	
}

DCF(min_ogl, "minimizes opengl")
{
	if (Dc_command)
	{
		if (gr_screen.mode != GR_OPENGL)
			return;
		opengl_minimize();
	}
	if (Dc_help)
		dc_printf("minimizes opengl\n");
}

DCF(aniso, "toggles anisotropic filtering")
{
	if (Dc_command)
	{
		if (max_aniso==1.0f) max_aniso=2.0f;
		else max_aniso=1.0f;
	}
	if (Dc_help) dc_printf("toggles anisotropic filtering");
}

#endif


