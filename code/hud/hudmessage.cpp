/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDmessage.cpp $
 * $Revision: 2.21 $
 * $Date: 2006-04-20 06:32:07 $
 * $Author: Goober5000 $
 *
 * C module that controls and manages the message window on the HUD
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.20  2006/03/19 05:05:58  taylor
 * make sure the mission log doesn't modify stuff in Cargo_names[], since it shouldn't
 * have split_str_once() be sure to not split a word in half, it should end up on the second line instead
 *
 * Revision 2.19  2006/01/13 03:30:59  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.18  2005/10/10 17:21:04  taylor
 * remove NO_NETWORK
 *
 * Revision 2.17  2005/07/25 08:21:59  Goober5000
 * more bugs and tweaks
 * --Goober5000
 *
 * Revision 2.16  2005/07/22 10:18:38  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.15  2005/07/18 03:44:01  taylor
 * cleanup hudtargetbox rendering from that total hack job that had been done on it (fixes wireframe view as well)
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.14  2005/07/13 03:15:52  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.13  2005/07/02 19:42:15  taylor
 * ton of non-standard resolution fixes
 *
 * Revision 2.12  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.11  2005/03/29 07:03:16  wmcoolmon
 * Removed some warnings under Linux/GCC
 *
 * Revision 2.10  2005/03/05 18:59:28  taylor
 * don't let hud messages walk off screen when res is GR_640
 *
 * Revision 2.9  2005/03/02 21:24:44  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.8  2005/02/23 04:51:56  taylor
 * some bm_unload() -> bm_release() changes to save bmpman slots
 *
 * Revision 2.7  2005/02/13 08:37:57  wmcoolmon
 * Made messages display properly in nonstandard resolutions
 *
 * Revision 2.6  2005/01/31 10:34:38  taylor
 * merge with Linux/OSX tree - p0131
 *
 * Revision 2.5  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:32:49  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.3  2004/03/05 09:02:03  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/03/18 10:07:03  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.1  2002/09/24 18:56:43  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/13 15:11:03  mharris
 * More NO_NETWORK ifndefs added
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 23    9/08/99 5:38p Jefff
 * 
 * 22    9/08/99 2:38p Jefff
 * sound pausing going to menu from game
 * 
 * 21    9/07/99 9:42p Jefff
 * 
 * 20    9/02/99 11:47a Jefff
 * yet another hud message length adjustment.  yeesh.
 * 
 * 19    8/25/99 10:08a Jefff
 * fixed another messge cut-off bug
 * 
 * 18    8/23/99 11:12a Jefff
 * fixed 1024 message cut-off bug
 * 
 * 17    8/20/99 2:26p Jefff
 * hud message text wrapping problem in hires fixed
 * 
 * 16    8/03/99 7:27p Jefff
 * hud messages go completely across screnn in high res now
 * 
 * 15    8/03/99 6:21p Jefff
 * fixed stupid bug with objectives screen key
 * 
 * 14    8/01/99 12:39p Dave
 * Added HUD contrast control key (for nebula).
 * 
 * 13    7/29/99 2:58p Jefff
 * Ingame objective screen icon key now uses normal objective icons and
 * text is drawn in code.
 * 
 * 12    6/16/99 5:26p Dave
 * Fixed some bitmap and coordinate problems on the mission scrollback
 * screen.
 * 
 * 11    6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 10    2/02/99 10:13a Neilk
 * fixed more coords
 * 
 * 9     2/01/99 5:55p Dave
 * Removed the idea of explicit bitmaps for buttons. Fixed text
 * highlighting for disabled gadgets.
 * 
 * 8     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 7     1/30/99 3:09p Neilk
 * More mission log coord fixes
 * 
 * 6     1/30/99 2:59p Neilk
 * Fixed more mission log coords
 * 
 * 5     1/29/99 7:57p Neilk
 * Added support for multiresolutions
 * 
 * 4     1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 89    6/09/98 5:17p Lawrance
 * French/German localization
 * 
 * 88    6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 87    5/19/98 8:36p Andsager
 * Fix bug where last line of message log would not show if more than one
 * screen of text.
 * 
 * 86    4/27/98 8:49p Allender
 * make terran command display in white (with correct code anyway).
 * 
 * 85    4/25/98 11:49p Lawrance
 * init pos to zero
 * 
 * 84    4/25/98 9:10p Hoffoss
 * Fixed bug with scrollback origin.
 * 
 * 83    4/25/98 5:22p Hoffoss
 * Fixed some problems with scrolling and positioning, and added code to
 * support pageup/pagedown.
 * 
 * 82    4/14/98 5:06p Dave
 * Don't load or send invalid pilot pics. Fixed chatbox graphic errors.
 * Made chatbox display team icons in a team vs. team game. Fixed up pause
 * and endgame sequencing issues.
 * 
 * 81    4/14/98 2:44p Hoffoss
 * Made arrow keys do what tab does.
 * 
 * 80    4/08/98 4:10p John
 * Removed all remaining traces of the evil gr_init_font_ex.
 * 
 * 79    4/07/98 11:33a Hoffoss
 * Fixed bug where scroll offset was wrong when first entering the F4
 * screen.
 * 
 * 78    4/05/98 3:30p Dave
 * Print netplayer messages in brighter green on the hud, with
 * accompanying sound. Echo netplayer messages on sending machine. Fixed
 * standalone sequencing bug where host never get the "enter mission"
 * button.
 * 
 * 77    3/27/98 11:57a Dave
 * Put in expression checking for text messages.
 * 
 * 76    3/17/98 4:01p Hoffoss
 * Added HUD_SOURCE_TERRAN_CMD and changed code to utilize it when a
 * message is being sent from Terran Command.
 * 
 * 75    3/16/98 5:55p Lawrance
 * Increase width of HUD message line, don't draw lines while comm menu is
 * up
 * 
 * 74    3/12/98 4:03p Hoffoss
 * Changed formatting used in hug scrollbacl log.
 * 
 * 73    3/09/98 4:47p Hoffoss
 * Changed F4 screen to start in objectives mode rather than HUD messages
 * mode.
 * 
 * 72    3/09/98 2:50p Hoffoss
 * Changed to use different palette file, and fixed bug with text
 * overrunning the right edge of screen.
 * 
 * 71    3/02/98 5:42p John
 * Removed WinAVI stuff from FreeSpace.  Made all HUD gauges wriggle from
 * afterburner.  Made gr_set_clip work good with negative x &y.  Made
 * model_caching be on by default.  Made each cached model have it's own
 * bitmap id.  Made asteroids not rotate when model_caching is on.  
 * 
 * 70    2/27/98 4:55p Hoffoss
 * Fixed some alignment problems.
 * 
 * 69    2/27/98 4:37p Hoffoss
 * Combined Objectives screen into Mission Log screen.
 * 
 * 68    2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 67    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 66    2/06/98 2:58p Hoffoss
 * Fixed bug where mission log scrolls twice when using the arrow keys.
 * 
 * 65    1/29/98 10:26a Hoffoss
 * Made changes so arrow buttons repeat scrolling when held down.
 * 
 * 64    1/20/98 4:39p Hoffoss
 * Added mission time display to scrollback log screen.
 * 
 * 63    1/18/98 5:09p Lawrance
 * Added support for TEAM_TRAITOR
 * 
 * 62    1/12/98 11:16p Lawrance
 * Wonderful HUD config.
 * 
 * 61    1/08/98 1:33p Hoffoss
 * Made scroll offset reset to bottom of list instead of top.
 * 
 * 60    1/05/98 2:59p Hoffoss
 * Fixed bug with messages drawing outside of limits.
 * 
 * 59    1/02/98 9:10p Lawrance
 * Big changes to how colors get set on the HUD.
 * 
 * 58    1/02/98 10:23a Hoffoss
 * Fixed incorrect scrolling directions in message scrollback log.
 * 
 * 57    12/11/97 10:17p Dave
 * Put in some checks to make sure HUD_printfs aren't
 * done in certain multiplayer situations.
 * 
 * 56    12/05/97 2:16p Hoffoss
 * Made hidden hud messages actually not show up in scrollback.
 * 
 * 55    12/03/97 6:07p Dave
 * Added assert that hud scrollback initialized before adding messages to
 * it.
 * 
 * 54    12/03/97 4:16p Hoffoss
 * Changed sound stuff used in interface screens for interface purposes.
 * 
 * 53    12/03/97 11:35a Hoffoss
 * Made changes to HUD messages send throughout the game.
 * 
 * 52    12/02/97 5:57p Hoffoss
 * Changed Hud messaging code to align text to right after sending ship's
 * name.
 * 
 * 51    12/01/97 4:30p Hoffoss
 * Changed code to list hud messages in scrollback from top down.
 * 
 * 50    11/25/97 10:01a Jasen
 * Remoced excess buttons from MessageLog screen.
 * 
 * 49    11/24/97 10:03p Jasen
 * Dang... had to make an entirely new revision of the last button.  :)
 * 
 * 48    11/24/97 9:44p Jasen
 * Changed button name and coords to new exit button.
 * 
 * 47    11/20/97 12:02p Lawrance
 * change Error to nprintf at warning level
 * 
 * 46    11/17/97 6:37p Lawrance
 * new gauges: extended target view, new lock triangles, support ship view
 * 
 * 45    11/14/97 2:46p Lawrance
 * decrease width of HUD message line to 435, so it doesn overlap with
 * message menu
 * 
 * 44    11/13/97 10:16p Hoffoss
 * Added icons to mission log scrollback.
 * 
 * 43    11/13/97 4:05p Hoffoss
 * Added hiding code for mission log entries.
 * 
 * 42    11/12/97 6:00p Hoffoss
 * Added training messages to hud scrollback log.
 * 
 * 41    11/11/97 11:16a Hoffoss
 * Changed hud scrollback to color entire lines.
 * 
 * 40    11/06/97 5:42p Hoffoss
 * Added support for fixed size timstamp rendering.
 * 
 * 39    11/05/97 7:11p Hoffoss
 * Made changed to the hud message system.  Hud messages can now have
 * sources so they can be color coded.
 * 
 * 38    11/04/97 4:56p Jasen
 * Updated coordinates for buttons
 * 
 * 37    11/03/97 10:12p Hoffoss
 * Finished up work on the hud message/mission log scrollback screen.
 * 
 * 36    11/03/97 5:38p Dave
 * Cleaned up more multiplayer sequencing. Added OBJ_OBSERVER module/type.
 * Restructured HUD_config structs/flags.
 * 
 * 35    10/25/97 4:02p Lawrance
 * took out unused hud_message struct members
 * 
 * 34    10/02/97 9:53p Hoffoss
 * Added event evaluation analysis debug screen so we can determine the
 * state of events and their sexp trees to track down logic problems and
 * such.
 * 
 * 33    9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 32    9/08/97 12:01p Lawrance
 * when re-using HUD scrollback entries, ensure memory gets free'ed
 * properly
 * 
 * 31    8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 30    8/22/97 10:03a Lawrance
 * fix exception that occurred when hud scrollback was selected from the
 * main menu
 * 
 * 29    6/23/97 12:03p Lawrance
 * move split_str() to Parselo
 * 
 * 28    6/17/97 12:25p Lawrance
 * HUD message lines are split into multiple lines when they exceed
 * display width
 * 
 * 27    6/12/97 10:23a John
 * added new colors to freespace.  made most menus display the background
 * bitmap rather than dull black screen.
 * 
 * 26    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 25    4/22/97 3:14p Lawrance
 * only free HUD scrollback messages if they exist
 * 
 * 24    4/15/97 1:26p Lawrance
 * using a static array of nodes to store hud scrollback messages, storage
 * for text is dynamic
 * 
 * 23    4/14/97 9:55a Mike
 * Fixed HUD message system.
 * Better game sequencing.
 * 
 * 22    1/28/97 5:33p Lawrance
 * saving number of msg window lines in save game and player file
 * 
 * 21    1/28/97 4:59p Lawrance
 * allowing number of lines on hud message bar to be configured
 * 
 * 20    1/24/97 9:47a Lawrance
 * made number of message lines in HUD message area confiurable
 * 
 * 19    1/22/97 10:56a Lawrance
 * added check for NULL after malloc()
 * 
 * 18    1/07/97 5:36p Lawrance
 * Enabled save/restore for  old/present/pending hud messages
 * 
 * 17    12/10/96 12:28p Lawrance
 * adding new offscreen target indicator
 * 
 * 16    12/08/96 1:54a Lawrance
 * integrating hud configuration
 * 
 * 15    11/29/96 6:12p Lawrance
 * took out duplicate include of timer.h
 * 
 * 14    11/29/96 11:17a Lawrance
 * added comments, put check in for zero-length HUD messages in the
 * scrollback
 * 
 * 13    11/28/96 6:27p Lawrance
 * added some additional comments
 * 
 * 12    11/27/96 3:20p Lawrance
 * added scroll-back message code
 * 
 * 11    11/22/96 1:00p Lawrance
 * fixed bug when a key held down and created tons of messages
 * 
 * 10    11/22/96 12:35p Lawrance
 * 
 * 9     11/20/96 11:51a Lawrance
 * trying out losing the HUD message shaded area
 * 
 * 8     11/19/96 4:46p Lawrance
 * fixed problem when too many messages were to be displayed at once
 * 
 * 7     11/19/96 3:55p Lawrance
 * modifed HUD message details (scroll speed, fade time etc)
 * 
 * 6     11/19/96 10:16a Lawrance
 * changing to new use of color scheme
 * 
 * 5     11/17/96 5:28p Lawrance
 * using HUD color globals instead of hard-coded numbers
 * 
 * 4     11/15/96 11:46a Lawrance
 * tweaked size of HUD message bar to prevent clipping of g's etc on the
 * bottom line
 * 
 * 3     11/15/96 11:39a Lawrance
 * got HUD messages scrolling
 * 
 * 2     11/15/96 12:11a Lawrance
 * HUD message bar working
 *
 * $NoKeywords: $
 *
*/

#include <stdlib.h>
#include <stdarg.h>


#include "hud/hud.h"
#include "hud/hudmessage.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "io/timer.h"
#include "playerman/player.h"
#include "globalincs/linklist.h"
#include "mission/missionlog.h"
#include "ui/ui.h"
#include "missionui/missionscreencommon.h"
#include "graphics/font.h"
#include "gamesnd/gamesnd.h"
#include "mission/missiongoals.h"
#include "globalincs/alphacolors.h"
#include "weapon/beam.h"
#include "sound/audiostr.h"
#include "ship/ship.h"
#include "parse/parselo.h"
#include "mission/missionmessage.h"		// for MAX_MISSION_MESSAGES
#include "iff_defs/iff_defs.h"
#include "network/multi.h"




/* replaced with those static ints that follow
#define LIST_X		46
#define LIST_X2	108  // second column x start position
#define LIST_Y		60
#define LIST_W		558  // total width including both columns
#define LIST_W2	(LIST_W + LIST_X - LIST_X2)  // width of second column
#define LIST_H		297
#define LIST_H_O	275  // applies only to objectives mode
*/

// 1st column, width includes both columns
static int Hud_mission_log_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		46,60,558,269		// GR_640
	},
	{
		74,96,558,297		// GR_1024
	}
};

// 2nd column, width is just of second column
static int Hud_mission_log_list2_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		108, 60, 496, 297		// GR_640
	},
	{
		136, 96, 496, 436		// GR_1024
	}
};

static int Hud_mission_log_list_objective_x_coord[GR_NUM_RESOLUTIONS] = {
	275,	// GR_640
	440	// GR_1024
};

static int Hud_mission_log_time_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		41, 372	// GR_640
	},
	{
		66, 595	// GR_1024
	}
};

static int Hud_mission_log_time2_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		103, 372	// GR_640
	},
	{
		128, 595	// GR_1024
	}
};


#define SCROLLBACK_MODE_MSGS_LOG		0
#define SCROLLBACK_MODE_EVENT_LOG	1
#define SCROLLBACK_MODE_OBJECTIVES	2

#define NUM_BUTTONS			6

#define SCROLL_UP_BUTTON	0
#define SCROLL_DOWN_BUTTON	1
#define SHOW_MSGS_BUTTON	2
#define SHOW_EVENTS_BUTTON	3
#define SHOW_OBJS_BUTTON	4
#define ACCEPT_BUTTON		5

#define HUD_MESSAGE_TOTAL_LIFE	14000	// total time a HUD message is alive (in milliseconds)

#define HUD_MSG_LENGTH_MAX		2048
//#define HUD_MSG_MAX_PIXEL_W	439	// maximum number of pixels wide message display area is
//#define HUD_MSG_MAX_PIXEL_W	619	// maximum number of pixels wide message display area is

/* not used anymore
static int Hud_mission_log_status_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		170, 339		// GR_640
	},
	{
		361, 542		// GR_1024
	}
};
*/

struct scrollback_buttons {
	char *filename;
	int x, y;
	int xt, yt;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	scrollback_buttons(char *name, int x1, int y1, int x2, int y2, int h) : filename(name), x(x1), y(y1), xt(x2), yt(y2), hotspot(h) {}
};

int Scroll_time_id;

int MSG_WINDOW_X_START = 5;
int MSG_WINDOW_Y_START = 5;
int MSG_WINDOW_WIDTH;		// initialed in hud_init_msg_window()
int MSG_WINDOW_HEIGHT;
int MSG_WINDOW_FONT_HEIGHT;
int ACTIVE_BUFFER_LINES	= 4;				// number of HUD messages that can be shown at one time + 1
int OLD_ACTIVE_BUFFER_LINES;

int Hud_list_start;			// points to the next msg to be printed in the queue
int Hud_list_end;				// points to the last msg in the queue

int Active_index=0;
int Scroll_needed=0;
int Scroll_in_progress=0;

HUD_message_data HUD_pending[SCROLL_BUFFER_LINES];
Hud_display_info HUD_active_msgs_list[MAX_ACTIVE_BUFFER_LINES];

int HUD_msg_inited = FALSE;

// There is a maximum number of lines that will be stored in the message scrollback.  Oldest
// messages are deleted to make way for newest messages.
// Goober5000 - I think it'd be nice to see *all* the messages
#define MAX_MSG_SCROLLBACK_LINES	MAX_MISSION_MESSAGES
//#define MAX_MSG_SCROLLBACK_LINES	100
line_node Msg_scrollback_lines[MAX_MSG_SCROLLBACK_LINES];

line_node Msg_scrollback_free_list;
line_node Msg_scrollback_used_list;

#define	MAX_HUD_FT	1

typedef struct HUD_ft {
	int	end_time;						//	Timestamp at which this message will go away.
	char	text[MAX_HUD_LINE_LEN];		//	Text to display.
	int	color;							//	0rgb color, 8 bit fields.
} HUD_ft;

HUD_ft HUD_fixed_text[MAX_HUD_FT];

static int Num_obj_lines;
static int Scroll_offset;
static int Scroll_max;
static int Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
// static int Status_bitmap;
static int Background_bitmap;
static UI_WINDOW Ui_window;

static char* Hud_mission_log_fname[GR_NUM_RESOLUTIONS] = {
	"MissionLog",		// GR_640
	"2_MissionLog"		// GR_1024
};

/* not used anymore
static char* Hud_mission_log_status_fname[GR_NUM_RESOLUTIONS] = {
	"MLStatus",		// GR_640
	"MLStatus"		// GR_1024
};
*/

static char* Hud_mission_log_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MissionLog-m",		// GR_640
	"2_MissionLog-m"		// GR_1024
};

static scrollback_buttons Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
	{	// GR_640
	//XSTR:OFF
		scrollback_buttons("LB_00",	1,		67,	-1,	-1,	0),
		scrollback_buttons("LB_01",	1,		307,	-1,	-1,	1),
		scrollback_buttons("LB_02",	111,	376,	108,	413,	2),
		scrollback_buttons("LB_03",	209,	376,	205,	413,	3),
		scrollback_buttons("LB_04",	12,	376,	7,		413,	4),
		scrollback_buttons("CB_05a",	571,	425,	564,	413,	5)
	//XSTR:ON
	},
	{	// GR_1024
	//XSTR:OFF
		scrollback_buttons("2_LB_00",	1,		108,	-1,	-1,	0),
		scrollback_buttons("2_LB_01",	1,		492,	-1,	-1,	1),
		scrollback_buttons("2_LB_02",	177,	602,	173,	661,	2),
		scrollback_buttons("2_LB_03",	335,	602,	335,	661,	3),
		scrollback_buttons("2_LB_04",	20,	602,	11,	661,	4),
		scrollback_buttons("2_CB_05a",914,	681,	946,	661,	5)
	//XSTR:ON
	}
}
;

// ----------------------------------------------------------------------
// HUD_init_fixed_text()
//
void HUD_init_fixed_text()
{
	int	i;

	for (i=0; i<MAX_HUD_FT; i++)
		HUD_fixed_text[i].end_time = timestamp(0);
}

// ----------------------------------------------------------------------
// hud_init_msg_window()
//
// Called from HUD_init(), which is called from game_level_init()
//
void hud_init_msg_window()
{
	int i, h;

	MSG_WINDOW_WIDTH = gr_screen.clip_width_unscaled - 20;

	Hud_list_start = 0;
	Hud_list_end = 0;

	for (i=0; i<SCROLL_BUFFER_LINES; i++)
		HUD_pending[i].text[0] = HUD_pending[i].text[MAX_HUD_LINE_LEN - 1] = 0;
	
	for ( i=0; i < MAX_ACTIVE_BUFFER_LINES; i++ ) {
		HUD_active_msgs_list[i].total_life = 1;
	}

	Scroll_time_id = 1;

	// determine the height of the msg window, which depends on the font height	
	gr_set_font(FONT1);
	h = gr_get_font_height();

	//ACTIVE_BUFFER_LINES = Players[Player_num].HUD_config.num_msg_window_lines;
//	ACTIVE_BUFFER_LINES = HUD_config.num_msg_window_lines;
	ACTIVE_BUFFER_LINES = 4;

	MSG_WINDOW_FONT_HEIGHT = h;
	MSG_WINDOW_HEIGHT = MSG_WINDOW_FONT_HEIGHT * (ACTIVE_BUFFER_LINES-1);

	// starting a mission, free the scroll-back buffers, but only if we've been
	// through this function once already
	if ( HUD_msg_inited == TRUE ) {
		hud_free_scrollback_list();
	}

	list_init( &Msg_scrollback_free_list );
	list_init( &Msg_scrollback_used_list );

	// first slot is reserved for dummy node
	for (i=1; i < MAX_MSG_SCROLLBACK_LINES; i++)	{
		Msg_scrollback_lines[i].text = NULL;
		list_append(&Msg_scrollback_free_list, &Msg_scrollback_lines[i]);
	}

	Active_index=0;
	Scroll_needed=0;
	Scroll_in_progress=0;

	OLD_ACTIVE_BUFFER_LINES = ACTIVE_BUFFER_LINES;

	HUD_init_fixed_text();
	HUD_msg_inited = TRUE;
}

// ---------------------------------------------------------------------------------------
// hud_show_msg_window() will display the active HUD messages on the HUD.  It will scroll
// the messages up when a new message arrives.  
//
void hud_show_msg_window()
{
	int i, index;

	hud_set_default_color();
	gr_set_font(FONT1);

	HUD_set_clip(MSG_WINDOW_X_START,MSG_WINDOW_Y_START, MSG_WINDOW_WIDTH, MSG_WINDOW_HEIGHT+2);

	if ( OLD_ACTIVE_BUFFER_LINES != ACTIVE_BUFFER_LINES ) {
		// the size of the message window has changed, the best thing to do is to put all
		// the blank out the current hud messages.  There is no need to add them to the 
		// scrollback buffer, since they are already there!
	
		for ( i=0; i < ACTIVE_BUFFER_LINES; i++ ) {
			if ( !timestamp_elapsed(HUD_active_msgs_list[i].total_life) ) {
				HUD_active_msgs_list[i].total_life = 1;
				Active_index=0;
			}
		}
	}
	
	OLD_ACTIVE_BUFFER_LINES = ACTIVE_BUFFER_LINES;

	// check if there is a message to display on the HUD, and if there is room to display it
	if ( Hud_list_start != Hud_list_end && !Scroll_needed) {

		Hud_list_start++;

		// if the pointer exceeds the array size, wrap around to element 1.  element 0 is not used.		
		if (Hud_list_start >= SCROLL_BUFFER_LINES)
			Hud_list_start = 1;

		HUD_active_msgs_list[Active_index].msg = HUD_pending[Hud_list_start];
		HUD_active_msgs_list[Active_index].total_life = timestamp(HUD_MESSAGE_TOTAL_LIFE);

		for (i=Active_index+1; i < Active_index+ACTIVE_BUFFER_LINES; i++) {
			index = i % ACTIVE_BUFFER_LINES;

			// determine if there are any existing messages, if so need to scroll them up

			if ( !timestamp_elapsed(HUD_active_msgs_list[index].total_life) ) {
				HUD_active_msgs_list[index].target_y -=  MSG_WINDOW_FONT_HEIGHT;
				Scroll_needed=1;
			}

		}

		if (Scroll_needed) {
			HUD_active_msgs_list[Active_index].y = (ACTIVE_BUFFER_LINES-1)*MSG_WINDOW_FONT_HEIGHT;
			HUD_active_msgs_list[Active_index].target_y = HUD_active_msgs_list[Active_index].y - MSG_WINDOW_FONT_HEIGHT;
		}
		else {
			HUD_active_msgs_list[Active_index].y = (ACTIVE_BUFFER_LINES-2)*MSG_WINDOW_FONT_HEIGHT;
			HUD_active_msgs_list[Active_index].target_y = HUD_active_msgs_list[Active_index].y;
		}

		Active_index++;
		if (Active_index >= ACTIVE_BUFFER_LINES) Active_index = 0;

		if (Hud_list_end == Hud_list_start) {	// just printed the last msg
			Hud_list_start = Hud_list_end = 0;
		}
	}

	Scroll_in_progress=0;
	Scroll_needed = 0;

	for ( i=0; i < ACTIVE_BUFFER_LINES; i++ ) {

		if ( !timestamp_elapsed(HUD_active_msgs_list[i].total_life) ) {

			if (HUD_active_msgs_list[i].y > HUD_active_msgs_list[i].target_y) {
				Scroll_needed=1;
				if (timestamp_elapsed(Scroll_time_id) ){
					HUD_active_msgs_list[i].y -= SCROLL_STEP_SIZE;
					if (HUD_active_msgs_list[i].y < HUD_active_msgs_list[i].target_y)
						HUD_active_msgs_list[i].y = HUD_active_msgs_list[i].target_y;

					Scroll_in_progress=1;
				}

			}

			if ( hud_gauge_active(HUD_MESSAGE_LINES) ) {
				if ( !(Player->flags & PLAYER_FLAGS_MSG_MODE) ) {
					// set the appropriate color					
					if(HUD_active_msgs_list[i].msg.source){
						hud_set_gauge_color(HUD_MESSAGE_LINES, HUD_C_BRIGHT);
					} else {
						hud_set_gauge_color(HUD_MESSAGE_LINES);
					}

					// print the message out
					gr_printf(MSG_WINDOW_X_START + HUD_active_msgs_list[i].msg.x - 2, HUD_active_msgs_list[i].y, "%s", HUD_active_msgs_list[i].msg.text);
				}
			}
		}

	} // end for

	if (Scroll_in_progress)
		Scroll_time_id = timestamp(SCROLL_TIME);

	HUD_reset_clip();
}

void hud_show_fixed_text()
{
	HUD_ft	*hp;

	hp = &HUD_fixed_text[0];

	if (!timestamp_elapsed(hp->end_time)) {
		//gr_set_color((hp->color >> 16) & 0xff, (hp->color >> 8) & 0xff, hp->color & 0xff);
		gr_printf(0x8000, MSG_WINDOW_Y_START + MSG_WINDOW_HEIGHT + 8, hp->text);
	}
}

//	Similar to HUD printf, but shows only one message at a time, at a fixed location.
void HUD_fixed_printf(float duration, char * format, ...)
{
	va_list	args;
	char		tmp[HUD_MSG_LENGTH_MAX];
	int		msg_length;

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state != NETGAME_STATE_IN_MISSION)){
		nprintf(("Network","HUD_fixed_printf bailing because not in multiplayer game play state\n"));
		return;
	}

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	msg_length = strlen(tmp);
	Assert(msg_length < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.

	if ( !msg_length ) {
		nprintf(("Warning", "HUD_fixed_printf ==> attempt to print a 0 length string in msg window\n"));
		return;

	} else if (msg_length > MAX_HUD_LINE_LEN - 1){
		nprintf(("Warning", "HUD_fixed_printf ==> Following string truncated to %d chars: %s\n",MAX_HUD_LINE_LEN,tmp));
	}

	if (duration == 0.0f){
		HUD_fixed_text[0].end_time = timestamp(-1);
	} else {
		HUD_fixed_text[0].end_time = timestamp((int) (1000.0f * duration));
	}

	strncpy(HUD_fixed_text[0].text, tmp, MAX_HUD_LINE_LEN - 1);
	HUD_fixed_text[0].color = 0xff0000;
}

//	Clear all pending text.
void HUD_fixed_printf_reset()
{
	HUD_init_fixed_text();
}


// --------------------------------------------------------------------------------------
// HUD_printf_line() 
//
//	Print a single line of text to the HUD.  We know that the text will fit on the screen,
// since that was taken care of in HUD_printf();
//
void HUD_printf_line(char *text, int source, int time = 0, int x = 0)
{
	Assert(text != NULL);

	// if the pointer exceeds the array size, wrap around to element 1.  element 0 is not used.		
	Hud_list_end++;
	if (Hud_list_end >= SCROLL_BUFFER_LINES)
		Hud_list_end = 1;

	if (Hud_list_end == Hud_list_start) {
		nprintf(("Warning", "HUD ==> Exceeded msg scroll buffer, discarding message %s\n", text));
		Hud_list_end--;
		if (Hud_list_end == 0)
			Hud_list_end = SCROLL_BUFFER_LINES - 1;
		return;
	}

	if ( strlen(text) > MAX_HUD_LINE_LEN - 1 ){
		nprintf(("Warning", "HUD_printf_line() ==> Following string truncated to %d chars: %s\n", MAX_HUD_LINE_LEN, text));
	}

	strncpy(HUD_pending[Hud_list_end].text, text, MAX_HUD_LINE_LEN - 1);
	HUD_pending[Hud_list_end].text[MAX_HUD_LINE_LEN - 1] = 0;
	HUD_pending[Hud_list_end].source = source;
	HUD_pending[Hud_list_end].time = time;
	HUD_pending[Hud_list_end].x = x;
}

// converts a TEAM_* define to a HUD_SOURCE_* define
int HUD_team_get_source(int team)
{
	return team + HUD_SOURCE_TEAM_OFFSET;
}

// converts a HUD_SOURCE_* define to a TEAM_* define
int HUD_source_get_team(int source)
{
	return source - HUD_SOURCE_TEAM_OFFSET;
}


void HUD_printf(char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];
	int len;

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->state != NETPLAYER_STATE_IN_MISSION)){
		nprintf(("Network","HUD_printf bailing because not in multiplayer game play state\n"));
		return;
	}

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	len = strlen(tmp);
	Assert(len < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.
	hud_sourced_print(HUD_SOURCE_COMPUTER, tmp);
}

void HUD_ship_sent_printf(int sh, char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];
	int len;

	sprintf(tmp, NOX("%s: "), Ships[sh].ship_name);
	len = strlen(tmp);
	Assert(len < HUD_MSG_LENGTH_MAX);

	va_start(args, format);
	vsprintf(tmp + len, format, args);
	va_end(args);

	len = strlen(tmp);
	Assert(len < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.
	hud_sourced_print(HUD_team_get_source(Ships[sh].team), tmp);
}

// --------------------------------------------------------------------------------------
// HUD_sourced_printf() 
//
// HUD_sourced_printf() has the same parameters as printf(), but displays the text as a scrolling
// message on the HUD.  Text is split into multiple lines if width exceeds msg display area
// width.  'source' is used to indicate who send the message, and is used to color code text.
//
void HUD_sourced_printf(int source, char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->state != NETPLAYER_STATE_IN_MISSION)){
		nprintf(("Network","HUD_sourced_printf bailing because not in multiplayer game play state\n"));
		return;
	}
	
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);
	Assert(strlen(tmp) < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.
	hud_sourced_print(source, tmp);
}

void hud_sourced_print(int source, char *msg)
{
	char *ptr, *str;
	//char *src_str, *msg_str;
	int sw, t, x, offset = 0;
	//int fudge = (gr_screen.res == GR_640) ? 15 : 50;		// prevents string from running off screen

	if ( !strlen(msg) ) {
		nprintf(("Warning", "HUD ==> attempt to print a 0 length string in msg window\n"));
		return;
	}

	// add message to the scrollback log first
	hud_add_msg_to_scrollback(msg, source, timestamp());

	ptr = strstr(msg, NOX(": ")) + 2;
	if (ptr) {
		gr_get_string_size(&sw, NULL, msg, ptr - msg);			// get width of the speaker field
		//if (sw < MSG_WINDOW_WIDTH - 20)
		offset = sw;
	}

	x = 0;
	t = timestamp();
	str = msg;

	//Because functions to get font size don't compensate for *actual* screen size
	int pretend_width = (gr_screen.res == GR_640) ? 620 : 1004;

	while ((ptr = split_str_once(str, pretend_width - x - 7)) != NULL) {		// the 7 is a fudge hack
		// make sure that split went ok, if not then bail
		if (ptr == str) {
			Int3();
			break;
		}

		HUD_printf_line(str, source, t, x);
		str = ptr;
		x = offset;
		t = 0;
	}

	HUD_printf_line(str, source, t, x);
}

int hud_query_scrollback_size()
{
	int count = 0, y_add = 0;
	int font_height = gr_get_font_height();
	line_node *ptr;

	if (EMPTY(&Msg_scrollback_used_list) || !HUD_msg_inited)
		return 0;

	ptr = GET_FIRST(&Msg_scrollback_used_list);
	while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
		if (ptr->source != HUD_SOURCE_HIDDEN) {
			y_add = ptr->y;
			count += font_height + ptr->y;
		}

		ptr = GET_NEXT(ptr);
	}

	count -= y_add;
	return count;
}

// add text directly to the hud scrollback log, without displaying on the hud
void HUD_add_to_scrollback(char *text, int source)
{
	if (!strlen(text)) {
		nprintf(("Warning", "HUD ==> attempt to print a 0 length string in msg window\n"));
		return;
	}

	hud_add_msg_to_scrollback(text, source, timestamp());
}

// hud_add_msg_to_scrollback() adds the new_msg to the scroll-back message list.  If there
// are no more free slots, the first slot is released to make room for the new message.
//
void hud_add_line_to_scrollback(char *text, int source, int t, int x, int y, int underline_width)
{
	line_node *new_line;

	Assert(HUD_msg_inited);
	if (!text || !strlen(text))
		return;

	if ( EMPTY(&Msg_scrollback_free_list) ) {
		new_line = GET_FIRST(&Msg_scrollback_used_list);
		list_remove(&Msg_scrollback_used_list, new_line);
		vm_free(new_line->text);

	} else {
		new_line = GET_FIRST(&Msg_scrollback_free_list);
		list_remove(&Msg_scrollback_free_list, new_line);
	}

	new_line->x = x;
	new_line->y = y;
	new_line->underline_width = underline_width;
	new_line->time = t;
	new_line->source = source;
	new_line->text = (char *) vm_malloc( strlen(text) + 1 );
	strcpy(new_line->text, text);
	list_append(&Msg_scrollback_used_list, new_line);
}

void hud_add_msg_to_scrollback(char *text, int source, int t)
{
	char buf[HUD_MSG_LENGTH_MAX], *ptr, *str;
	int msg_len, w, max_width, x, offset = 0;

	max_width = Hud_mission_log_list2_coords[gr_screen.res][2];
	msg_len = strlen(text);
	if (msg_len == 0)
		return;

	w = 0;
	Assert(msg_len < HUD_MSG_LENGTH_MAX);
	strcpy(buf, text);
	ptr = strstr(buf, NOX(": "));
	if (ptr) {
		gr_get_string_size(&w, NULL, buf, ptr - buf);
	}

//	if (ptr) {
//		gr_get_string_size(&w, NULL, buf, ptr - buf + 2);
//		if (w < max_width - 20)
//			offset = w;
//	}

	x = 0;
	str = buf;
	while ((ptr = split_str_once(str, max_width - x)) != NULL) {
		hud_add_line_to_scrollback(str, source, t, x, 1, w);
		str = ptr;
		x = offset;
		t = w = 0;
	}

	hud_add_line_to_scrollback(str, source, t, x, 3, w);
}

// hud_free_scrollback_list() will free the memory that was allocated to store the messages
// for the scroll-back list
//
void hud_free_scrollback_list()
{
	line_node *A;

	// check if the list has been inited yet.  If not, return with doing nothing.
	if ( Msg_scrollback_used_list.next == NULL || Msg_scrollback_used_list.prev == NULL )
		return;

	A = GET_FIRST(&Msg_scrollback_used_list);
	while( A !=END_OF_LIST(&Msg_scrollback_used_list) )	{
		if ( A->text != NULL ) {
			vm_free(A->text);
			A->text = NULL;
		}

		A = GET_NEXT(A);
	}
}

// how many lines to skip
int hud_get_scroll_max_pos()
{
	int max = 0, font_height = gr_get_font_height();

	if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
		int count = 0;
		line_node *ptr;
		// number of pixels in excess of what can be displayed
		int excess = Scroll_max - Hud_mission_log_list_coords[gr_screen.res][3];

		if (EMPTY(&Msg_scrollback_used_list) || !HUD_msg_inited) {
			max = 0;

		} else {
			ptr = GET_FIRST(&Msg_scrollback_used_list);
			while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
				if (ptr->source != HUD_SOURCE_HIDDEN) {

					if (excess > 0) {
						excess -= font_height;
						count++;
					}

					if (excess <= 0) {
						max = count;
						break;
					}

					// spacing between lines
					excess -= ptr->y;

				}

				ptr = GET_NEXT(ptr);
			}
		}

	} else {
		max = (Scroll_max - Hud_mission_log_list_coords[gr_screen.res][3]) / font_height;
	}

	if (max < 0)
		max = 0;

	return max;
}

void hud_scroll_reset()
{
	if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
		Scroll_offset = 0;

	} else {
		Scroll_offset = hud_get_scroll_max_pos();
	}
}

void hud_scroll_list(int dir)
{
	if (dir) {
		if (Scroll_offset) {
			Scroll_offset--;
			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);

	} else {
		if (Scroll_offset < hud_get_scroll_max_pos()) {
			Scroll_offset++;
			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void hud_goto_pos(int delta)
{
	int pos=0, font_height = gr_get_font_height();

	if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
		int count = 0, y = 0;
		line_node *ptr;

		if (EMPTY(&Msg_scrollback_used_list) || !HUD_msg_inited)
			return;

		ptr = GET_FIRST(&Msg_scrollback_used_list);
		while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
			if (ptr->source != HUD_SOURCE_HIDDEN) {
				if (count == Scroll_offset) {
					pos = y;
					break;
				}

				y += font_height + ptr->y;
				count++;
			}

			ptr = GET_NEXT(ptr);
		}

		Scroll_offset = count = y = 0;
		ptr = GET_FIRST(&Msg_scrollback_used_list);
		while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
			if (ptr->source != HUD_SOURCE_HIDDEN) {
				if (y <= pos + delta)
					Scroll_offset = count;

				y += font_height + ptr->y;
				count++;
			}

			ptr = GET_NEXT(ptr);
		}

	} else {
		pos = Scroll_offset * font_height;
		pos += delta;
		Scroll_offset = pos / font_height;
	}
}

void hud_page_scroll_list(int dir)
{
	int max = hud_get_scroll_max_pos();

	if (dir) {
		if (Scroll_offset) {
			hud_goto_pos(-Hud_mission_log_list_coords[gr_screen.res][3]);
			if (Scroll_offset < 0)
				Scroll_offset = 0;

			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);

	} else {
		if (Scroll_offset < max) {
			hud_goto_pos(Hud_mission_log_list_coords[gr_screen.res][3]);
			if (Scroll_offset > max)
				Scroll_offset = max;

			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void hud_scrollback_button_pressed(int n)
{
	switch (n) {
		case SCROLL_UP_BUTTON:
			hud_scroll_list(1);
			break;

		case SCROLL_DOWN_BUTTON:
			hud_scroll_list(0);
			break;

		case SHOW_MSGS_BUTTON:
			Scrollback_mode = SCROLLBACK_MODE_MSGS_LOG;
			Scroll_max = hud_query_scrollback_size();
			hud_scroll_reset();
			break;

		case SHOW_EVENTS_BUTTON:
			Scrollback_mode = SCROLLBACK_MODE_EVENT_LOG;
			Scroll_max = Num_log_lines * gr_get_font_height();
			hud_scroll_reset();
			break;

		case SHOW_OBJS_BUTTON:
			Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
			Scroll_max = Num_obj_lines * gr_get_font_height();
			Scroll_offset = 0;
			break;

		case ACCEPT_BUTTON:
			hud_scrollback_exit();			
			break;
	}
}

void hud_scrollback_init()
{
	int i;
	scrollback_buttons *b;

	// pause all game sounds
	beam_pause_sounds();
	audiostream_pause_all();

	common_set_interface_palette("BriefingPalette");  // set the interface palette
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Hud_mission_log_mask_fname[gr_screen.res]);

	for (i=0; i<NUM_BUTTONS; i++) {
		b = &Buttons[gr_screen.res][i];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, (i < 2), 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add all strings	
	Ui_window.add_XSTR("Continue", 1069, Buttons[gr_screen.res][ACCEPT_BUTTON].xt,  Buttons[gr_screen.res][ACCEPT_BUTTON].yt, &Buttons[gr_screen.res][ACCEPT_BUTTON].button, UI_XSTR_COLOR_PINK);
	Ui_window.add_XSTR("Events", 1070, Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].xt,  Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].yt, &Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Objectives", 1071, Buttons[gr_screen.res][SHOW_OBJS_BUTTON].xt,  Buttons[gr_screen.res][SHOW_OBJS_BUTTON].yt, &Buttons[gr_screen.res][SHOW_OBJS_BUTTON].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Messages", 1072, Buttons[gr_screen.res][SHOW_MSGS_BUTTON].xt,  Buttons[gr_screen.res][SHOW_MSGS_BUTTON].yt, &Buttons[gr_screen.res][SHOW_MSGS_BUTTON].button, UI_XSTR_COLOR_GREEN);

	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	Buttons[gr_screen.res][SCROLL_UP_BUTTON].button.set_hotkey(KEY_UP);
	Buttons[gr_screen.res][SCROLL_DOWN_BUTTON].button.set_hotkey(KEY_DOWN);

	Background_bitmap = bm_load(Hud_mission_log_fname[gr_screen.res]);
	// Status_bitmap = bm_load(Hud_mission_log_status_fname[gr_screen.res]);

	message_log_init_scrollback(Hud_mission_log_list_coords[gr_screen.res][2]);
	if (Scrollback_mode == SCROLLBACK_MODE_EVENT_LOG)
		Scroll_max = Num_log_lines * gr_get_font_height();
	else if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES)
		Scroll_max = Num_obj_lines * gr_get_font_height();
	else
		Scroll_max = hud_query_scrollback_size();

	Num_obj_lines = ML_objectives_init(Hud_mission_log_list_coords[gr_screen.res][0], Hud_mission_log_list_coords[gr_screen.res][1], Hud_mission_log_list_coords[gr_screen.res][2], Hud_mission_log_list_objective_x_coord[gr_screen.res]);
	hud_scroll_reset();
}

void hud_scrollback_close()
{
	ML_objectives_close();
	message_log_shutdown_scrollback();
	if (Background_bitmap >= 0)
		bm_release(Background_bitmap);
	//if (Status_bitmap >= 0)
	//	bm_unload(Status_bitmap);

	Ui_window.destroy();
	common_free_interface_palette();		// restore game palette
	game_flush();

	// unpause all game sounds
	beam_unpause_sounds();
	audiostream_unpause_all();

}

void hud_scrollback_do_frame(float frametime)
{
	int i, k, x, y;
	int font_height = gr_get_font_height();

	k = Ui_window.process();
	switch (k) {
		case KEY_RIGHT:
		case KEY_TAB:
			if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
				Scrollback_mode = SCROLLBACK_MODE_MSGS_LOG;
				Scroll_max = hud_query_scrollback_size();
				hud_scroll_reset();

			} else if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
				Scrollback_mode = SCROLLBACK_MODE_EVENT_LOG;
				Scroll_max = Num_log_lines * gr_get_font_height();
				hud_scroll_reset();

			} else {
				Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
				Scroll_max = Num_obj_lines * gr_get_font_height();
				Scroll_offset = 0;
			}

			break;

		case KEY_LEFT:
		case KEY_SHIFTED | KEY_TAB:
			if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
				Scrollback_mode = SCROLLBACK_MODE_EVENT_LOG;
				Scroll_max = Num_log_lines * gr_get_font_height();
				hud_scroll_reset();

			} else if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
				Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
				Scroll_max = Num_obj_lines * gr_get_font_height();
				Scroll_offset = 0;

			} else {
				Scrollback_mode = SCROLLBACK_MODE_MSGS_LOG;
				Scroll_max = hud_query_scrollback_size();
				hud_scroll_reset();
			}

			break;

		case KEY_PAGEUP:
			hud_page_scroll_list(1);
			break;

		case KEY_PAGEDOWN:
			hud_page_scroll_list(0);
			break;

		case KEY_ENTER:
		case KEY_CTRLED | KEY_ENTER:
		case KEY_ESC:			
			hud_scrollback_exit();
			break;

		case KEY_F1:  // show help overlay
			break;

		case KEY_F2:  // goto options screen
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);
			break;
	}	// end switch

	for (i=0; i<NUM_BUTTONS; i++){
		if (Buttons[gr_screen.res][i].button.pressed()){
			hud_scrollback_button_pressed(i);		
		}
	}

	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0);
	}

	/*
	if ((Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) && (Status_bitmap >= 0)) {
		gr_set_bitmap(Status_bitmap);
		gr_bitmap(Hud_mission_log_status_coords[gr_screen.res][0], Hud_mission_log_status_coords[gr_screen.res][1]);
	}
	*/

	// draw the objectives key at the bottom of the ingame objectives screen
	if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
		ML_render_objectives_key();
	}

	Ui_window.draw();

	if (Scrollback_mode == SCROLLBACK_MODE_EVENT_LOG) {
		Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].button.draw_forced(2);
		mission_log_scrollback(Scroll_offset, Hud_mission_log_list_coords[gr_screen.res][0], Hud_mission_log_list_coords[gr_screen.res][1], Hud_mission_log_list_coords[gr_screen.res][2], Hud_mission_log_list_coords[gr_screen.res][3]);

	} else if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
		Buttons[gr_screen.res][SHOW_OBJS_BUTTON].button.draw_forced(2);
		ML_objectives_do_frame(Scroll_offset);

	} else {
		line_node *node_ptr;

		Buttons[gr_screen.res][SHOW_MSGS_BUTTON].button.draw_forced(2);
//		y = ((LIST_H / font_height) - 1) * font_height;
		y = 0;
		if ( !EMPTY(&Msg_scrollback_used_list) && HUD_msg_inited ) {
			node_ptr = GET_FIRST(&Msg_scrollback_used_list);
			i = 0;
			while ( node_ptr != END_OF_LIST(&Msg_scrollback_used_list) ) {
				if ((node_ptr->source == HUD_SOURCE_HIDDEN) || (i++ < Scroll_offset)) {
					node_ptr = GET_NEXT(node_ptr);

				} else {
					int team = HUD_source_get_team(node_ptr->source);

					if (team >= 0)
					{
						gr_set_color_fast(iff_get_color_by_team(team, Player_ship->team, 0));
					}
					else
					{
						switch (node_ptr->source)
						{
							case HUD_SOURCE_TRAINING:
								gr_set_color_fast(&Color_bright_blue);
								break;
	
							case HUD_SOURCE_TERRAN_CMD:
								gr_set_color_fast(&Color_bright_white);
								break;
	
							case HUD_SOURCE_IMPORTANT:
							case HUD_SOURCE_FAILED:
							case HUD_SOURCE_SATISFIED:
								gr_set_color_fast(&Color_bright_white);
								break;
	
							default:
								gr_set_color_fast(&Color_text_normal);
								break;
						}
					}

					if (node_ptr->time)
						gr_print_timestamp(Hud_mission_log_list_coords[gr_screen.res][0], Hud_mission_log_list_coords[gr_screen.res][1] + y, node_ptr->time);

					x = Hud_mission_log_list2_coords[gr_screen.res][0] + node_ptr->x;
					gr_printf(x, Hud_mission_log_list_coords[gr_screen.res][1] + y, "%s", node_ptr->text);
					if (node_ptr->underline_width)
						gr_line(x, Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height - 1, x + node_ptr->underline_width, Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height - 1);

					if ((node_ptr->source == HUD_SOURCE_FAILED) || (node_ptr->source == HUD_SOURCE_SATISFIED)) {
						// draw goal icon
						if (node_ptr->source == HUD_SOURCE_FAILED)
							gr_set_color_fast(&Color_bright_red);
						else
							gr_set_color_fast(&Color_bright_green);

						i = Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height / 2 - 1;
						gr_circle(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i, 5);

						gr_set_color_fast(&Color_bright);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 10, i, Hud_mission_log_list2_coords[gr_screen.res][0] - 8, i);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i - 4, Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i - 2);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 4, i, Hud_mission_log_list2_coords[gr_screen.res][0] - 2, i);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i + 2, Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i + 4);
					}

					y += font_height + node_ptr->y;
					node_ptr = GET_NEXT(node_ptr);
					if (y + font_height > Hud_mission_log_list_coords[gr_screen.res][3])
						break;
				}
			}
		}
	}

	gr_set_color_fast(&Color_text_heading);
	gr_print_timestamp(Hud_mission_log_time_coords[gr_screen.res][0], Hud_mission_log_time_coords[gr_screen.res][1] - font_height, (int) (f2fl(Missiontime) * 1000));
	gr_string(Hud_mission_log_time2_coords[gr_screen.res][0], Hud_mission_log_time_coords[gr_screen.res][1] - font_height, XSTR( "Current time", 289));
	gr_flip();
}

void hud_scrollback_exit()
{
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}
