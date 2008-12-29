// TabCommLine.cpp : implementation file
//

#include <io.h>
#include <direct.h>

#include "stdafx.h"
#include "Launcher.h"
#include "TabCommLine.h"

#include "win32func.h"
#include "misc.h"
#include "settings.h"

#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"

#include "LauncherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	"CUSTOM DONT CHANGE", NULL, NULL,					FLAG_MULTI | FLAG_SCP,
};

typedef struct
{
	char name[32];

} EasyFlag;

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


int num_eflags = 0;
int num_params = 0;

EasyFlag *easy_flags  = NULL;
Flag	 *exe_params  = NULL;
bool	 *flag_states = NULL;

#define MAX_CMDLINE_SIZE		3000
#define MAX_CUSTOM_PARAM_SIZE	1200

char command_line[MAX_CMDLINE_SIZE]   = "";

/////////////////////////////////////////////////////////////////////////////
// CTabCommLine dialog


CTabCommLine::CTabCommLine(CWnd* pParent /*=NULL*/)
	: CDialog(CTabCommLine::IDD, pParent)
{
	m_flag_gen_in_process = false;

	//{{AFX_DATA_INIT(CTabCommLine)
	//}}AFX_DATA_INIT
}

CTabCommLine::~CTabCommLine()
{
	if(easy_flags)	free(easy_flags);
	if(exe_params)	free(exe_params);
	if(flag_states)	free(flag_states);
}

void CTabCommLine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabCommLine)
	DDX_Control(pDX, IDC_FLAG_TYPE, m_flag_type_list);
	DDX_Control(pDX, IDC_FLAG_SETUP, m_easy_flag);
	DDX_Control(pDX, IDC_FLAG_LIST, m_flag_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabCommLine, CDialog)
	//{{AFX_MSG_MAP(CTabCommLine)
	ON_EN_CHANGE(IDC_CUSTOM_PARAM, OnChangeCustomParam)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FLAG_LIST, OnItemchangedFlagList)
	ON_CBN_SELCHANGE(IDC_FLAG_SETUP, OnSelchangeFlagSetup)
	ON_NOTIFY(NM_DBLCLK, IDC_FLAG_LIST, OnDblclkFlagList)
	ON_CBN_SELCHANGE(IDC_FLAG_TYPE, OnSelchangeFlagType)
	ON_BN_CLICKED(IDC_SETTINGS_NORMAL, OnSettingsNormal)
	ON_BN_CLICKED(IDC_SETTINGS_MOD, OnSettingsMod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCommLine message handlers

BOOL CTabCommLine::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// Make sure its a checkbox and insert a column 
	m_flag_list.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
 	m_flag_list.InsertColumn(0, "Flag", LVCFMT_LEFT, 200);
	return TRUE;  
}

/**
 *
 *
 * @param char *new_path
 */
void CTabCommLine::UpdateFields()
{
	if(Settings::exe_path_valid == false)
	{
		GetDlgItem(IDC_COMM_LINE_PATH)->SetWindowText("");
		return;
	}

	strcpy(command_line, Settings::exe_filepath);

	if (Settings::exe_type != EXE_TYPE_CUSTOM) {
		GetDlgItem(IDC_SETTINGS_NORMAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_SETTINGS_MOD)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMM_LINE_PATH)->SetWindowText(command_line);   
		CLauncherDlg::Redraw();
		return;
	}

	char mod_param[MAX_PATH * 3];
	tab_mod.GetModCommandLine(mod_param);
	if(strlen(mod_param) > 0)
	{
 		strcat(command_line, mod_param);
		GetDlgItem(IDC_SETTINGS_NORMAL)->EnableWindow(TRUE);
		GetDlgItem(IDC_SETTINGS_MOD)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_SETTINGS_NORMAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_SETTINGS_MOD)->EnableWindow(FALSE);
	}

	char standard_param[MAX_PATH * 3];
	UpdateStandardParam(standard_param);	
	if(strlen(standard_param) > 0)
	{
 		strcat(command_line, " ");
 		strcat(command_line, standard_param);
	}

	CString custom_param;
	GetDlgItem(IDC_CUSTOM_PARAM)->GetWindowText(custom_param);
	if(strlen(custom_param) > 0)
	{
 		strcat(command_line, " ");
 		strcat(command_line, custom_param);
	}


	GetDlgItem(IDC_COMM_LINE_PATH)->SetWindowText(command_line);   
	CLauncherDlg::Redraw();
}

/**
 * When the custom parameter edit box is typed into reflect this change in the
 * final command line box
 */
void CTabCommLine::OnChangeCustomParam() 
{
	UpdateFields();	
}

/**
 * @return CString - string holding command line (may be empty)
 */
CString CTabCommLine::GetCommLine()
{
	return CString(command_line);
}

/**
 * @return int - type of exe (one of following)
 *
 * EXE_TYPE_NONE
 * EXE_TYPE_CUSTOM
 * An integer to reference the 'exe_types' array with
 */
void CTabCommLine::SelectRegPathAndExeType()
{
	if(Settings::exe_path_valid == false ||
	   !file_exists(Settings::exe_filepath))
	{
		Settings::set_reg_path("", EXE_TYPE_NONE);
		return;
	}

	int exe_type = EXE_TYPE_CUSTOM;

	// Use filename and size to determine official builds
	for(int i = 0; i < MAX_EXE_TYPES; i++)
	{
		// Confirm this by name
		if(stricmp(exe_types[i].exe_name, Settings::exe_nameonly) == 0)
		{
			exe_type = i;
			break;
		}
	}

	char reg_path[MAX_PATH];

	if(exe_type < MAX_EXE_TYPES)
	{
		sprintf(reg_path, "SOFTWARE\\%s\\%s", 
			exe_types[exe_type].company, 
			exe_types[exe_type].regname);
	}
	else
	{
		sprintf(reg_path, "SOFTWARE\\%s\\%s", 
				exe_types[EXE_TYPE_FS2].company,
				exe_types[EXE_TYPE_FS2].regname);
	}
			
	Settings::set_reg_path(reg_path, exe_type);
}

/**
 *  This is called when a tick box is ticked or unticked on the parameter list
 */
void CTabCommLine::OnItemchangedFlagList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = 0;

	if(pNMListView->iItem < 0) return;
	if(m_flag_gen_in_process == true) return;

 	int index			= m_flag_list.GetItemData(pNMListView->iItem);
 	flag_states[index]	= (m_flag_list.GetCheck(pNMListView->iItem) != 0);

	UpdateFields();
}

/**
 * This takes the standard parameter choices the user has made and turns them into a string
 */
void CTabCommLine::UpdateStandardParam(char *standard_param)
{
	strcpy(standard_param, "");

	for(int i = 0; i < num_params; i++)
	{
		if(flag_states[i] == true)
		{
			strcat(standard_param, exe_params[i].name);		
			strcat(standard_param, " ");		
		}
	}	 
}

void CTabCommLine::ConstructFlagListRetail()
{
	int i, count = 0;
	char last_word[FLAG_TYPE_LEN] = "";

	if ( exe_types[Settings::exe_type].flags & FLAG_FS1 )
		num_params = Num_retail_params_FS1;
	else
		num_params = Num_retail_params_FS2;

	exe_params  = (Flag *) malloc(sizeof(Flag) * num_params);
	flag_states = (bool *) malloc(sizeof(bool) * num_params);

	if ( (exe_params == NULL) || (flag_states == NULL) )
		return;

	if ( exe_types[Settings::exe_type].flags & FLAG_FS1 )
		memcpy( exe_params, retail_params_FS1, sizeof(Flag) * num_params );
	else
		memcpy( exe_params, retail_params_FS2, sizeof(Flag) * num_params );

	memset( flag_states, 0, sizeof(bool) * num_params );

	// setup flag types
	m_flag_type_list.ResetContent();
	m_flag_type_list.EnableWindow(TRUE);

	for (i = 0; i < num_params; i++) {
		if ( strcmp(exe_params[i].type, last_word) ) {
			m_flag_type_list.InsertString(count, exe_params[i].type);
			strcpy(last_word, exe_params[i].type);
			count++;
		}
	}

	m_flag_type_list.SetCurSel(0);
}

/**
 *
 */
void CTabCommLine::ConstructFlagList()
{
	// If this is a retail FS2 exe skip all this
	if (Settings::exe_type != EXE_TYPE_CUSTOM) {
		m_easy_flag.ResetContent();
		m_easy_flag.EnableWindow(FALSE);

		ConstructFlagListRetail();
		ConstructFlagListInternal();

		return;
	}

	m_flag_type_list.EnableWindow(TRUE);
	m_flag_list.EnableWindow(TRUE);
	m_easy_flag.EnableWindow(TRUE);

	// Delete and generate new flag file
	char flag_file[MAX_PATH];
	sprintf(flag_file, "%s\\flags.lch",Settings::exe_pathonly);

	DeleteFile(flag_file);

	if ( !run_file((LPTSTR) Settings::exe_nameonly, (LPTSTR) Settings::exe_pathonly," -get_flags", true) )
		return;

	FILE *fp = fopen(flag_file, "r");

	int focount = 2000;
	while(fp == NULL && focount)
	{
		fp = fopen(flag_file, "r");
		focount--;
	}

	if(fp == NULL)
	{
		MessageBox("Failed to read flag file", "Fatal Error", MB_OK);
		return;
	}

	int eflags_struct_size;
	int params_struct_size;

	fread(&eflags_struct_size, sizeof(int), 1, fp);
	fread(&params_struct_size, sizeof(int), 1, fp);

	if(eflags_struct_size != sizeof(EasyFlag) ||
	   params_struct_size != sizeof(Flag))
	{
		MessageBox("Launcher and fs2_open versions do not work with each other", "Fatal Error", MB_OK);
		fclose(fp);
		DeleteFile(flag_file);
		return;
	}

	if(easy_flags)
		free(easy_flags);
	if(exe_params)
		free(exe_params);
	if(flag_states)
		free(flag_states);

	fread(&num_eflags, sizeof(int), 1, fp);
	easy_flags = (EasyFlag *) malloc(sizeof(EasyFlag) * num_eflags); 

	if(easy_flags == NULL)
	{
		MessageBox("Failed to allocate enough memory for easy_flags", "Fatal Error", MB_OK);
		fclose(fp);
		DeleteFile(flag_file);
		return;
	}  

	fread(easy_flags, sizeof(EasyFlag) * num_eflags, 1, fp);
	
	fread(&num_params, sizeof(int), 1, fp);
	exe_params  = (Flag *) malloc(sizeof(Flag) * num_params); 
	flag_states = (bool *) malloc(sizeof(bool) * num_params);

	memset(flag_states, 0, sizeof(bool) * num_params);

	if(exe_params == NULL || flag_states == NULL)
	{
		MessageBox("Failed to allocate enough memory for exe_params", "Fatal Error", MB_OK);
		fclose(fp);
		DeleteFile(flag_file);
		return;
	}

	fread(exe_params, sizeof(Flag) * num_params, 1, fp);

	// hack for OpenAL check
	if ( (filelength(fileno(fp)) - ftell(fp)) == 1 ) {
		Settings::is_openal(true);
	}

	fclose(fp);

	// go ahead and delete the file now, it's just taking up space at this point
	DeleteFile(flag_file);

	// Setup Easy Flags
	m_easy_flag.ResetContent();
	for(int k = 0; k < num_eflags; k++)
	{
		m_easy_flag.InsertString(k,easy_flags[k].name);
	}

	// Setup Flag Types
	m_flag_type_list.ResetContent();

	int count = 0;
	char last_word[FLAG_TYPE_LEN] = "";
	for(k = 0; k < num_params; k++)
	{
		if(strcmp(exe_params[k].type, last_word) != 0)
		{
			m_flag_type_list.InsertString(count, exe_params[k].type);
			strcpy(last_word, exe_params[k].type);
			count++;

			if(count > 19)
			{
				MessageBox("Flag type count is 20 or more, please report to coder","Error");
				count = 19;
				break;
			}
		}
	}

	m_flag_type_list.SetCurSel(0);
	ConstructFlagListInternal();
}

void CTabCommLine::ConstructFlagListInternal()
{
	int type_index = m_flag_type_list.GetCurSel();

	char type_text[FLAG_TYPE_LEN];
	m_flag_type_list.GetWindowText(type_text, FLAG_TYPE_LEN);

	// Setup flags
	m_flag_list.DeleteAllItems();
	m_flag_list.EnableWindow(TRUE);

	int exe_flags = 0;

	// If we are not sure what this exe is give access to all parameters
	if(Settings::exe_type != EXE_TYPE_NONE)
	{
		exe_flags = exe_types[Settings::exe_type].flags;
	}

	m_flag_gen_in_process = true;

	// For each parameter
	for(int i = 0, j = 0; i < num_params; i++)
	{
		if(strcmp(exe_params[i].type, type_text) != 0)
			continue;

		if(exe_params[i].fso_only && !(exe_types[Settings::exe_type].flags & FLAG_FS2OPEN))
		  	continue;

		// Insert this item but keep a record of the index number
		// The insert item index will not stay as is and will mess up if there are gaps (unshown params)
		if(strlen(exe_params[i].desc) > 0)
			m_flag_list.InsertItem(j, exe_params[i].desc, 0);
		else
			m_flag_list.InsertItem(j, exe_params[i].name, 0);

	  	m_flag_list.SetItemData(j, i);
		j++;
	}

	m_flag_gen_in_process = false;
}

bool check_cfg_file(char *dest_buffer, bool create_dir = false)
{
	strcpy(dest_buffer, Settings::exe_pathonly);
	strcat(dest_buffer, "\\data");

	if (create_dir) {
		_mkdir(dest_buffer);
	}

	if (Settings::exe_type != EXE_TYPE_CUSTOM) {
		strcat(dest_buffer, "\\cmdline.cfg");
	} else {
		strcat(dest_buffer, "\\cmdline_fso.cfg");
	}
			
	return file_exists(dest_buffer);
}

/**
 *
 */
bool CTabCommLine::SaveSettings()
{
	FILE *fp = NULL;
	char path_buffer[MAX_PATH];
	char standard_param[MAX_PATH];
	char mod_param[MAX_PATH];
	char custom_param[MAX_CUSTOM_PARAM_SIZE] = {'\0'};

	GetDlgItem(IDC_CUSTOM_PARAM)->GetWindowText(custom_param, MAX_CUSTOM_PARAM_SIZE);

	UpdateStandardParam(standard_param);

	check_cfg_file(path_buffer, true);

	// if not a custom exe then it must be a retail exe, in which case just save to the cfg
	if ( Settings::exe_type != EXE_TYPE_CUSTOM ) {
		if (strlen(standard_param) > 0) {
			strcat(custom_param, " ");
			strcat(custom_param, standard_param);
		}
	
		fp = fopen(path_buffer, "wt");
			
		if (fp) {
			fwrite(custom_param, strlen(custom_param) * sizeof(char), 1, fp);
			fclose(fp);
			fp = NULL;
		}

		return true;
	}

	tab_mod.SetSettings(standard_param);
	tab_mod.GetActiveModName(mod_param);

	if (strlen(standard_param) > 0) {
		strcat(custom_param, " ");
		strcat(custom_param, standard_param);
	}

	fp = ini_open_for_write(Settings::ini_main, true, NULL);

	if (fp) {
		ini_write_data(fp, "game_flags", custom_param);
		ini_write_data(fp, "active_mod", mod_param);

		ini_close(fp);
		fp = NULL;
	}

	// Write the mod details for the cfg file
	tab_mod.GetModCommandLine(mod_param);		
	if (strlen(mod_param) > 0) {
		strcat(custom_param, mod_param);
	}

	// Make the cfg file
	fp = fopen(path_buffer, "wt");

	if (fp) {
		fwrite(custom_param, strlen(custom_param) * sizeof(char), 1, fp);
		fclose(fp);
	}

	return true;
}

/**
 * @return - EXE_TYPE_NONE, EXE_TYPE_CUSTOM or if recognised exe it returns the flag list.
 */
int CTabCommLine::GetFlags()
{
	if(Settings::exe_type == EXE_TYPE_NONE) 
		return 0;

	return exe_types[Settings::exe_type].flags;
}

void CTabCommLine::LoadSettings(char *reg_path)
{
	char custom_param[MAX_CUSTOM_PARAM_SIZE] = "";

	// a non-retail exe will grab options from the launcher ini
	if (Settings::exe_type == EXE_TYPE_CUSTOM) {
		int exe_flags = GetFlags();

		dictionary *ini = iniparser_load(Settings::ini_main);

		if (ini == NULL) 
			return;

		iniparser_dump(ini, stderr);
	
		if (exe_flags & FLAG_MOD) {
			char *mod = iniparser_getstr(ini, "launcher:active_mod");
			SetModParam(mod);
		}

		char *flags	= iniparser_getstr(ini, "launcher:game_flags");

		if (flags)
			strcpy(custom_param, flags);

		iniparser_freedict(ini);
	}
	// a retail exe will just grab options from the game cfg
	else {
		FILE *fp = NULL;
		char path_buffer[MAX_PATH];

		check_cfg_file(path_buffer);

		fp = fopen(path_buffer, "rt");
			
		if (fp) {
			fgets(custom_param, sizeof(custom_param) - 1, fp);
			fclose(fp);
			fp = NULL;
		}
	}

	char *end_string_here = NULL;
	char *found_str = NULL;
	size_t get_new_offset = 0;

	// Seperate custom flags from standard one
	for(int i = 0; i < num_params; i++) {
		// while going through the cmdline make sure to grab only the option that we
		// are looking for, but if one similar then keep searching for the exact match
		do {
			found_str = strstr(&custom_param[0] + get_new_offset, exe_params[i].name);

			if (found_str && (*(found_str + strlen(exe_params[i].name))) && (*(found_str + strlen(exe_params[i].name)) != ' ') ) {
				// the new offset should be our current location + the length of the current option
				get_new_offset = (strlen(custom_param) - strlen(found_str) + strlen(exe_params[i].name));
			} else {
				get_new_offset = 0;
			}
		} while ( get_new_offset );

		if (found_str == NULL)
			continue;

		flag_states[i] = true;

		if (end_string_here == NULL)
			end_string_here = found_str;
	}

	// Cut the standard options out of the custom string
	if (end_string_here != NULL) {
	   	if (end_string_here > custom_param && end_string_here[-1] == ' ') {
			end_string_here[-1] = '\0';
		} else {
			end_string_here[0] = '\0';
		}
	}

	GetDlgItem(IDC_CUSTOM_PARAM)->SetWindowText(custom_param);
	UpdateFlagList();
}	  

void CTabCommLine::SetModParam(char *path)
{
	if(path == NULL)
	{
		return;
	}

	char absolute_path[MAX_PATH];
	strcpy(absolute_path, Settings::exe_pathonly);
	strcat(absolute_path, "\\");
	strcat(absolute_path, path);

   	tab_mod.SetMOD(absolute_path);
	UpdateFields();
	return;
}

// User has selected new easy select type
void CTabCommLine::OnSelchangeFlagSetup() 
{				   
  	int easy_flag = m_easy_flag.GetCurSel();

	// Custom setting, leave it alone
	if(easy_flag == 0) return;

	for(int i = 0; i < num_params; i++)
	{
		if(exe_params[i].on_flags & (1 << easy_flag))
			flag_states[i] = true;

		if(exe_params[i].off_flags & (1 << easy_flag))
			flag_states[i] = false;
	}

   	UpdateFlagList();
   	UpdateFields();
}

// User has selected a new flag group type
void CTabCommLine::OnSelchangeFlagType() 
{
  	ConstructFlagListInternal();
	UpdateFlagList();
}

void CTabCommLine::UpdateFlagList()
{
	int num_flags = m_flag_list.GetItemCount();

	for(int i = 0; i < num_flags; i++)
	{
		int index = m_flag_list.GetItemData(i);
		m_flag_list.SetCheck(i, flag_states[index]);
	}

	CLauncherDlg::Redraw();
}

// User has double clicked flag
void CTabCommLine::OnDblclkFlagList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	if(exe_params == NULL) return;
	if(pNMListView->iItem < 0) return;

	int index = m_flag_list.GetItemData(pNMListView->iItem);

	if(index < 0 || index > num_params)
	{
		MessageBox("Bad m_flag_list index value","Error");
	}

	if(strlen(exe_params[index].web_url) > 0)
	{
		if(exe_params[index].web_url[0] == 'h' && 
		   exe_params[index].web_url[1] == 't' && 
		   exe_params[index].web_url[2] == 't' && 
		   exe_params[index].web_url[3] == 'p')
		{
			open_web_page(exe_params[index].web_url);
		}
		else
		{
			MessageBox(exe_params[index].web_url,"Info");
		}
	}
	else
		MessageBox("No online help for this feature","Sorry");
}


void CTabCommLine::OnSettingsNormal() 
{
	// TODO: Add your control notification handler code here
	UpdateFields();
	
}

void CTabCommLine::OnSettingsMod() 
{
	// TODO: Add your control notification handler code here
	tab_mod.GetSettings(true);
	UpdateFields();	

}
