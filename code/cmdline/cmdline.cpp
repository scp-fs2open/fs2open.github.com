/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Cmdline/cmdline.cpp $
 * $Revision: 2.9 $
 * $Date: 2002-10-27 23:55:36 $
 * $Author: DTP $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2002/10/22 23:02:39  randomtiger
 * Made Phreaks alternative scanning style optional under the command line tag "-phreak"
 * Fixed bug that changes HUD colour when targetting debris in a full nebula. - RT
 *
 * Revision 2.7  2002/10/19 03:50:28  randomtiger
 * Added special pause mode for easier action screenshots.
 * Added new command line parameter for accessing all single missions in tech room. - RT
 *
 * Revision 2.6  2002/08/27 13:38:57  penguin
 * Moved DirectX8 stuff to directx8 branch; reverted to previous
 *
 * Revision 2.4  2002/08/07 00:44:13  DTP
 * Implented -GF4FIX commandline switch
 *
 * Revision 2.3  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/29 06:35:15  DTP
 * added -almission commandline arguement, will autoload mission i.e fs2_open.exe -almission kickass will autoload kickass.fs2 which should be a multiplayer mission.
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/08 17:29:17  mharris
 * more port tweaks
 *
 * Revision 1.2  2002/05/07 13:22:14  mharris
 * added pstypes.h include
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 7     7/15/99 3:07p Dave
 * 32 bit detection support. Mouse coord commandline.
 * 
 * 6     7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 5     6/22/99 9:37p Dave
 * Put in pof spewing.
 * 
 * 4     1/12/99 5:45p Dave
 * Moved weapon pipeline in multiplayer to almost exclusively client side.
 * Very good results. Bandwidth goes down, playability goes up for crappy
 * connections. Fixed object update problem for ship subsystems.
 * 
 * 3     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 38    10/02/98 3:22p Allender
 * fix up the -connect option and fix the -port option
 * 
 * 37    9/15/98 4:04p Allender
 * added back in the -ip_addr command line switch because it needs to be
 * in the standalone server only executable
 * 
 * 36    9/14/98 11:52a Allender
 * don't use cfile
 * 
 * 35    9/14/98 11:28a Allender
 * support for server bashing of address when received from client.  Added
 * a cmdline.cfg file to process command line arguments from a file
 * 
 * 34    9/08/98 2:20p Allender
 * temporary code to force IP address to a specific value.
 * 
 * 33    8/20/98 5:30p Dave
 * Put in handy multiplayer logfile system. Now need to put in useful
 * applications of it all over the code.
 * 
 * 32    8/07/98 10:39a Allender
 * fixed debug standalone problem where stats would continually get sent
 * to tracker.  more debug code to help find stats problem
 * 
 * 31    7/24/98 11:14a Allender
 * start of new command line options for version 1.04
 * 
 * 30    5/21/98 1:50a Dave
 * Remove obsolete command line functions. Reduce shield explosion packets
 * drastically. Tweak PXO screen even more. Fix file xfer system so that
 * we can guarantee file uniqueness.
 * 
 * 29    5/18/98 9:10p Dave
 * Put in many new PXO features. Fixed skill level bashing in multiplayer.
 * Removed several old command line options. Put in network config files.
 * 
 * 28    5/09/98 7:16p Dave
 * Put in CD checking. Put in standalone host password. Made pilot into
 * popup scrollable.
 * 
 * 27    4/23/98 8:27p Allender
 * basic support for cutscene playback.  Into movie code in place.  Tech
 * room can view cutscenes stored in CDROM_dir variable
 * 
 * 26    4/09/98 5:43p Dave
 * Remove all command line processing from the demo. Began work fixing up
 * the new multi host options screen.
 * 
 * 25    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 24    4/01/98 5:56p Dave
 * Fixed a messaging bug which caused msg_all mode in multiplayer not to
 * work. Compile out a host of multiplayer options not available in the
 * demo.
 * 
 * 23    3/14/98 2:48p Dave
 * Cleaned up observer joining code. Put in support for file xfers to
 * ingame joiners (observers or not). Revamped and reinstalled pseudo
 * lag/loss system.
 * 
 * 22    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 21    1/31/98 4:32p Dave
 * Put in new support for VMT player validation, game logging in and game
 * logging out.
 * 
 * 20    12/10/97 4:45p Dave
 * Added in more detailed support for multiplayer packet lag/loss. Fixed
 * some multiplayer stuff. Added some controls to the standalone.
 * 
 * 19    12/09/97 6:14p Lawrance
 * add -nomusic flag
 * 
 * 18    12/01/97 5:10p Dave
 * Fixed a syntax bug.
 * 
 * 17    12/01/97 4:59p Dave
 * Synchronized multiplayer debris objects. Put in pilot popup in main
 * hall. Optimized simulated multiplayer lag module. Fixed a potential
 * file_xfer bug.
 * 
 * 16    11/28/97 7:04p Dave
 * Emergency checkin due to big system crash.
 * 
 * 15    11/28/97 5:06p Dave
 * Put in facilities for simulating multiplayer lag.
 * 
 * 14    11/24/97 5:42p Dave
 * Fixed a file xfer buffer free/malloc problem. Lengthened command line
 * switch string parse length.
 * 
 * 13    11/12/97 4:39p Dave
 * Put in multiplayer campaign support parsing, loading and saving. Made
 * command-line variables better named. Changed some things on the initial
 * pilot select screen.
 * 
 * 12    11/11/97 4:54p Dave
 * Put in support for single vs. multiplayer pilots. Put in initial player
 * selection screen (no command line option yet). Started work on
 * multiplayer campaign file save gaming.
 * 
 * 11    11/11/97 11:55a Allender
 * initialize network at beginning of application.  create new call to set
 * which network protocol to use
 * 
 * 10    9/18/97 10:12p Dave
 * Added -gimmemedals, which gives the current pilot all the medals in the
 * game (debug)
 * 
 * 9     9/18/97 9:20a Dave
 * Minor modifications
 * 
 * 8     9/15/97 11:40p Lawrance
 * remove demo granularity switch
 * 
 * 7     9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 6     9/03/97 5:03p Lawrance
 * add support for -nosound command line parm
 * 
 * 5     8/22/97 8:52a Dave
 * Removed a return statement which would have broken the parser out too
 * early.
 * 
 * 4     8/21/97 4:55p Dave
 * Added a switch for multiplayer chat streaming. Added a section for
 * global command line vars.
 * 
 * 3     8/06/97 2:26p Dave
 * Made the command line parse more robust. Made it easier to add and
 * process new command-line switches.
 * 
 * 2     8/04/97 3:13p Dave
 * Added command line functions. See cmdline.cpp for directions on adding
 * new switches
 * 
 * 1     8/04/97 9:58a Dave
 * 
 * $NoKeywords: $
 */

#include <string.h>
#include <stdlib.h>
#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"
#include "globalincs/linklist.h"
#include "globalincs/systemvars.h"
#include "network/multi.h"
#include "cfile/cfile.h"

// variables
class cmdline_parm {
public:
	cmdline_parm *next, *prev;
	char *name;						// name of parameter, must start with '-' char
	char *help;						// help text for this parameter
	char *args;						// string value for parameter arguements (NULL if no arguements)
	int name_found;				// true if parameter on command line, otherwise false

	cmdline_parm(char *name, char *help);
	~cmdline_parm();
	int found();
	int get_int();
	float get_float();
	char *str();
};

// here are the command line parameters that we will be using for FreeSpace
cmdline_parm standalone_arg("-standalone", NULL);
cmdline_parm nosound_arg("-nosound", NULL);
cmdline_parm nomusic_arg("-nomusic", NULL);
cmdline_parm startgame_arg("-startgame", NULL);
cmdline_parm gamename_arg("-gamename", NULL);
cmdline_parm gamepassword_arg("-password", NULL);
cmdline_parm gameclosed_arg("-closed", NULL);
cmdline_parm gamerestricted_arg("-restricted", NULL);
cmdline_parm allowabove_arg("-allowabove", NULL);
cmdline_parm allowbelow_arg("-allowbelow", NULL);
cmdline_parm port_arg("-port", NULL);
cmdline_parm connect_arg("-connect", NULL);
cmdline_parm multilog_arg("-multilog", NULL);
cmdline_parm server_firing_arg("-oldfire", NULL);
cmdline_parm client_dodamage("-clientdamage", NULL);
cmdline_parm pof_spew("-pofspew", NULL);
cmdline_parm d3d_32bit("-32bit", NULL);
cmdline_parm mouse_coords("-coords", NULL);
cmdline_parm timeout("-timeout", NULL);
cmdline_parm d3d_window("-window", NULL);
cmdline_parm almission_arg("-almission", NULL); //DTP for autoload Multi mission
cmdline_parm gf4fix_arg("-GF4FIX", NULL); //DTP for random tigers GF4fix
cmdline_parm allslev_arg("-ALLSLEV", NULL); //Give access to all single player missions
cmdline_parm phreak_arg("-phreak", NULL); // Change to phreaks options including new targetting code
cmdline_parm mod_arg("-mod", NULL); //DTP modsupport

int Cmdline_multi_stream_chat_to_file = 0;
int Cmdline_freespace_no_sound = 0;
int Cmdline_freespace_no_music = 0;
int Cmdline_gimme_all_medals = 0;
int Cmdline_use_last_pilot = 0;
int Cmdline_multi_protocol = -1;
int Cmdline_cd_check = 1;
int Cmdline_start_netgame = 0;
int Cmdline_closed_game = 0;
int Cmdline_restricted_game = 0;
int Cmdline_network_port = -1;
char *Cmdline_game_name = NULL;
char *Cmdline_game_password = NULL;
char *Cmdline_rank_above= NULL;
char *Cmdline_rank_below = NULL;
char *Cmdline_connect_addr = NULL;
char *Cmdline_almission = NULL;	//DTP for autoload multi mission.
char *Cmdline_mod = NULL; //DTP for mod arguement
int Cmdline_multi_log = 0;
int Cmdline_server_firing = 0;
int Cmdline_client_dodamage = 0;
int Cmdline_spew_pof_info = 0;
int Cmdline_force_32bit = 0;
int Cmdline_mouse_coords = 0;
int Cmdline_timeout = -1;

int Cmdline_window = 0;
int Cmdline_gf4fix = 0; // DTP for randomstigers GF4 fix.
int Cmdline_allslev = 0;
int Cmdline_phreak	= 0;
char *Hold_mod = NULL;



static cmdline_parm Parm_list(NULL, NULL);
static int Parm_list_inited = 0;


//	Return true if this character is an extra char (white space and quotes)
int is_extra_space(char ch)
{
	return ((ch == ' ') || (ch == '\t') || (ch == 0x0a) || (ch == '\'') || (ch == '\"'));
}


// eliminates all leading and trailing extra chars from a string.  Returns pointer passed in.
char *drop_extra_chars(char *str)
{
	int s, e;

	s = 0;
	while (str[s] && is_extra_space(str[s]))
		s++;

	e = strlen(str) - 1;
	while (e > s) {
		if (!is_extra_space(str[e])){
			break;
		}

		e--;
	}

	if (e > s){
		memmove(str, str + s, e - s + 1);
	}

	str[e - s + 1] = 0;
	return str;
}


// internal function - copy the value for a parameter agruement into the cmdline_parm arg field
void parm_stuff_args(cmdline_parm *parm, char *cmdline)
{
	char buffer[1024];
	memset(buffer, 0, 1024);
	char *dest = buffer;

	cmdline += strlen(parm->name);

	while ((*cmdline != 0) && (*cmdline != '-')) {
		*dest++ = *cmdline++;
	}

	drop_extra_chars(buffer);

	// mwa 9/14/98 -- made it so that newer command line arguments found will overwrite
	// the old arguments
//	Assert(parm->args == NULL);
	if ( parm->args != NULL ) {
		delete( parm->args );
		parm->args = NULL;
	}

	int size = strlen(buffer) + 1;
	if (size > 0) {
		parm->args = new char[size];
		memset(parm->args, 0, size);
		strcpy(parm->args, buffer);
	}
}


// internal function - parse the command line, extracting parameter arguements if they exist
// cmdline - command line string passed to the application
void os_parse_parms(char *cmdline)
{
	// locate command line parameters
	cmdline_parm *parmp;
	char *cmdline_offset;

	for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
		cmdline_offset = strstr(cmdline, parmp->name);
		if (cmdline_offset) {
			parmp->name_found = 1;
			parm_stuff_args(parmp, cmdline_offset);
		}
	}
}


// validate the command line parameters.  Display an error if an unrecognized parameter is located.
void os_validate_parms(char *cmdline)
{
	cmdline_parm *parmp;
	char seps[] = " ,\t\n";
	char *token;
	int parm_found;

   token = strtok(cmdline, seps);
   while(token != NULL) {
	
		if (token[0] == '-') {
			parm_found = 0;
			for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
				if (!stricmp(parmp->name, token)) {
					parm_found = 1;
					break;
				}
			}

			if (parm_found == 0) {
				Error(LOCATION,"Unrecogzined command line parameter %s", token);
			}
		}

		token = strtok(NULL, seps);
	}
}


// Call once to initialize the command line system
//
// cmdline - command line string passed to the application
void os_init_cmdline(char *cmdline)
{
	FILE *fp;

	// read the cmdline.cfg file from the data folder, and pass the command line arguments to
	// the the parse_parms and validate_parms line.  Read these first so anything actually on
	// the command line will take precedence
	fp = fopen("data\\cmdline.cfg", "rt");

	// if the file exists, get a single line, and deal with it
	if ( fp ) {
		char buf[1024], *p;

		fgets(buf, 1024, fp);

		// replace the newline character with a NUL:
		if ( (p = strrchr(buf, '\n')) != NULL ) {
			*p = '\0';
		}

		os_parse_parms(buf);
		os_validate_parms(buf);
		fclose(fp);
	}


	os_parse_parms(cmdline);
	os_validate_parms(cmdline);
}


// arg constructor
// name_ - name of the parameter, must start with '-' character
// help_ - help text for this parameter
cmdline_parm::cmdline_parm(char *name_, char *help_)
{
	name = name_;
	help = help_;
	args = NULL;
	name_found = 0;

	if (Parm_list_inited == 0) {
		list_init(&Parm_list);
		Parm_list_inited = 1;
	}

	if (name != NULL) {
		list_append(&Parm_list, this);
	}
}


// destructor - frees any allocated memory
cmdline_parm::~cmdline_parm()
{
	if (args) {
		delete [] args;
		args = NULL;
	}
}


// returns - true if the parameter exists on the command line, otherwise false
int cmdline_parm::found()
{
	return name_found;
}


// returns - the interger representation for the parameter arguement
int cmdline_parm::get_int()
{
	Assert(args);
	return atoi(args);
}


// returns - the float representation for the parameter arguement
float cmdline_parm::get_float()
{
	Assert(args);
	return (float)atof(args);
}


// returns - the string value for the parameter arguement
char *cmdline_parm::str()
{
	Assert(args);
	return args;
}

// external entry point into this modules
#ifdef _WIN32
int parse_cmdline(char *cmdline)
#else
int parse_cmdline(int argc, char *argv[])
#endif
{
   #ifdef _WIN32
	os_init_cmdline(cmdline);
   #else
	if (argc > 1) {
		// kind of silly -- combine arg list into single string for parsing,
		// but it fits with the win32-centric existing code.
		char *cmdline = NULL;
		unsigned int arglen = 0;
		int i;
		for (i = 1;  i < argc;  i++)
			arglen += strlen(argv[i]);
		if (argc > 2)
			arglen += argc - 2; // leave room for the separators
		cmdline = new char [arglen+1];
		i = 1;
		strcpy(cmdline, argv[i]);
		for ( ; i < argc;  i++) {
			strcat(cmdline, " ");
			strcat(cmdline, argv[i]);
		}
		os_init_cmdline(cmdline);
		delete [] cmdline;
	} else {
		// no cmdline args
		os_init_cmdline("");
	}
   #endif

	// is this a standalone server??
	if (standalone_arg.found()) {
		Is_standalone = 1;
	}

	// run with no sound
	if ( nosound_arg.found() ) {
		Cmdline_freespace_no_sound = 1;
	}

	// run with no music
	if ( nomusic_arg.found() ) {
		Cmdline_freespace_no_music = 1;
	}

	// should we start a network game
	if ( startgame_arg.found() ) {
		Cmdline_use_last_pilot = 1;
		Cmdline_start_netgame = 1;
	}

	// closed network game
	if ( gameclosed_arg.found() ) {
		Cmdline_closed_game = 1;
	}

	// restircted network game
	if ( gamerestricted_arg.found() ) {
		Cmdline_restricted_game = 1;
	}

	// get the name of the network game
	if ( gamename_arg.found() ) {
		Cmdline_game_name = gamename_arg.str();

		// be sure that this string fits in our limits
		if ( strlen(Cmdline_game_name) > MAX_GAMENAME_LEN ) {
			Cmdline_game_name[MAX_GAMENAME_LEN-1] = '\0';
		}
	}

	// get the password for a pssword game
	if ( gamepassword_arg.found() ) {
		Cmdline_game_password = gamepassword_arg.str();

		// be sure that this string fits in our limits
		if ( strlen(Cmdline_game_name) > MAX_PASSWD_LEN ) {
			Cmdline_game_name[MAX_PASSWD_LEN-1] = '\0';
		}
	}

	// set the rank above/below arguments
	if ( allowabove_arg.found() ) {
		Cmdline_rank_above = allowabove_arg.str();
	}
	if ( allowbelow_arg.found() ) {
		Cmdline_rank_below = allowbelow_arg.str();
	}

	// get the port number for games
	if ( port_arg.found() ) {
		Cmdline_network_port = port_arg.get_int();
	}

	// the connect argument specifies to join a game at this particular address
	if ( connect_arg.found() ) {
		Cmdline_use_last_pilot = 1;
		Cmdline_connect_addr = connect_arg.str();
	}

	// see if the multilog flag was set
	if ( multilog_arg.found() ){
		Cmdline_multi_log = 1;
	}	

	// maybe use old-school server-side firing
	if (server_firing_arg.found() ){
		Cmdline_server_firing = 1;
	}

	// maybe use old-school client damage
	if(client_dodamage.found()){
		Cmdline_client_dodamage = 1;
	}	

	// spew pof info
	if(pof_spew.found()){
		Cmdline_spew_pof_info = 1;
	}

	// 32 bit
	if(d3d_32bit.found()){
		Cmdline_force_32bit = 1;
	}

	// mouse coords
	if(mouse_coords.found()){
		Cmdline_mouse_coords = 1;
	}

	// net timeout
	if(timeout.found()){
		Cmdline_timeout = timeout.get_int();
	}

	// d3d windowed
	if(d3d_window.found()){
		Cmdline_window = 1;
	}
	if(almission_arg.found()){//DTP for autoload mission // developer oritentated
		Cmdline_almission = almission_arg.str();
		Cmdline_use_last_pilot = 1;
		Cmdline_start_netgame = 1;
	}
	if (gf4fix_arg.found() ) {
		Cmdline_gf4fix = 1;
	}

	if (allslev_arg.found() ) {
		Cmdline_allslev = 1;
	}

	if(phreak_arg.found() ) {
		Cmdline_phreak = 1;
	}

	if(mod_arg.found() ) {
		Cmdline_mod = mod_arg.str();
		Hold_mod = mod_arg.str();
	}

	return 1;
}
