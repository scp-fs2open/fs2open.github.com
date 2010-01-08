#include "metadata.h"



typedef struct
{
	char *exe_name;
	char *company;
	char *regname;
	int   flags;

} ExeType;				  

char *exe_types_string[MAX_EXE_TYPES + 2] = 
{
	"Official FreeSpace 2",     
	"Official FreeSpace 2 Demo",
	"Official FreeSpace 1"
	"No valid exe selected",
	"Custom exe (assuming fs2_open exe)",
	// Insert new exe data here and in exe_types
};

// This array stored specific information about the various exe's the launcher may be asked to deal with
ExeType exe_types[MAX_EXE_TYPES + 2] = 
{
//	exe name	   size     Reg dir 1	Reg dir 2			Flags
	"FS2.exe",     "Volition", "FreeSpace2",		FLAG_MULTI | FLAG_D3D5 | FLAG_FS2 | FLAG_3DFX, 
	"FS2Demo.exe", "Volition", "FreeSpace2Demo",	FLAG_MULTI | FLAG_D3D5 | FLAG_FS2 | FLAG_3DFX, 
	"FS.exe",	   "Volition", "FreeSpace", 		FLAG_MULTI | FLAG_D3D5 | FLAG_FS1 | FLAG_3DFX | FLAG_SFT,	
	"Placeholder", NULL,		NULL,				0,
	// Insert new exe data here and in exe_types_string
	"CUSTOM DON'T CHANGE", NULL, NULL,					FLAG_MULTI | FLAG_SCP,
};

const int FLAG_TYPE_LEN = 16;

typedef struct
{
	char  name[20];				// The actual flag
	char  desc[40];				// The text that will appear in the launcher
	bool  fso_only;				// true if this is a fs2_open only feature
	int   on_flags;				// Easy flag which will turn this feature on
	int   off_flags;			// Easy flag which will turn this feature off
	char  type[FLAG_TYPE_LEN];	// Launcher uses this to put flags under different headings
	char  web_url[256];			// Link to documentation of feature (please use wiki or somewhere constant)

} Flag;

Flag retail_params_FS2[] = {
	{ "-32bit",			"Enable D3D 32-bit mode",			false,	0,	2,	"Graphics",		"", },

	{ "-nosound",		"Disable sound and music",			false,	0,	2,	"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nosound", },
	{ "-nomusic",		"Disable music",					false,	0,	2,	"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomusic", },

	{ "-standalone",	"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-standalone", },
	{ "-startgame",		"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-startgame", },
	{ "-closed",		"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-closed", },
	{ "-restricted",	"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-restricted", },
	{ "-multilog",		"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-multilog", },
	{ "-clientdamage",	"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-clientdamage", },

	{ "-oldfire",		"",									false,	0,	2,	"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-oldfire", },

	{ "-coords",		"Show coordinates",					false,	0,	2,	"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-coords", },
	{ "-pofspew",		"",									false,	0,	2,	"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-pofspew", }
};

int Num_retail_params_FS2 = sizeof(retail_params_FS2) / sizeof(Flag);

Flag retail_params_FS1[] = {
	{ "-nosound",		"Disable sound and music",			false,	0,	2,	"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nosound", },
	{ "-nomusic",		"Disable music",					false,	0,	2,	"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomusic", },

	{ "-standalone",	"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-standalone", },
	{ "-startgame",		"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-startgame", },
	{ "-closed",		"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-closed", },
	{ "-restricted",	"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-restricted", },
	{ "-multilog",		"",									false,	0,	2,	"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-multilog", },
};

int Num_retail_params_FS1 = sizeof(retail_params_FS1) / sizeof(Flag);

