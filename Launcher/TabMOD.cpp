// TabMOD.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "TabMOD.h"
#include "win32func.h"
#include "LauncherDlg.h"
#include "misc.h"
#include "launcher_settings.h"
#include "mod_settings.h"

#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabMOD dialog


CTabMOD::CTabMOD(CWnd* pParent /*=NULL*/)
	: CDialog(CTabMOD::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabMOD)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	MOD_bitmap = NULL;
	m_absolute_text[0] = '\0';

	for(int i = 0; i < INI_MAX; i++)
	{
		ini_text[i] = NULL;
	} 

	mod_selected = false;
}

void CTabMOD::OnDestroy() 
{
	::DeleteObject (MOD_bitmap);
	CDialog::OnDestroy();

	for(int i = 0; i < INI_MAX; i++)
	{
		if(ini_text[i] != NULL)
		{
			free(ini_text[i]);
			ini_text[i] = NULL;
		}
	} 
}


void CTabMOD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabMOD)
	DDX_Control(pDX, IDC_MOD_IMAGE, m_mod_image);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabMOD, CDialog)
	//{{AFX_MSG_MAP(CTabMOD)
	ON_BN_CLICKED(IDC_MOD_SELECT, OnModSelect)
	ON_BN_CLICKED(IDC_MOD_NONE, OnModNone)
	ON_BN_CLICKED(IDC_MOD_WEBSITE, OnModWebsite)
	ON_BN_CLICKED(IDC_MOD_FORUM, OnModForum)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabMOD message handlers

void CTabMOD::OnModSelect() 
{
	char absolute_path[MAX_PATH];

	if (LauncherSettings::get_showed_pilot_warning())
	{
		MessageBox("To prevent pilot file corruption, it is a good idea to create a separate pilot for each mod you play.  Also, please remember to select a campaign in the Campaign Room that will work with your mod.\n\nThis message will not be shown again.  Have fun!");
		LauncherSettings::set_showed_pilot_warning(true);
	}

	if(browse_for_dir(GetSafeHwnd(), LauncherSettings::get_exe_pathonly(), absolute_path, 
		"Select a FreeSpace 2 mod directory") == false)
	{
		return;
	}

	if(strlen(absolute_path) == 0)
	{  
		MessageBox("Not a valid mod directory");
		return;
	}

/*	if(strchr(absolute_path,' '))
	{
		MessageBox("Mod directory name can't contain space");
		return;
	}*/

	if(stricmp(LauncherSettings::get_exe_pathonly(), absolute_path) == 0)
	{
		MessageBox("Cannot choose root");
		return;
	}

	SetMOD(absolute_path);
}

void CTabMOD::SetSettings(char *flags) 
{
	if(mod_selected == false)
	{
		return;
	}

	if(ModSettings::save_custom())
	{
		MessageBox("Couldnt write mod settings.ini");
	}
}

char *CTabMOD::GetSettings(bool defaultSettings) 
{
	Int3();
	/*
	char ini_name[MAX_PATH];
	strcpy(ini_name,m_absolute_text);

	if(defaultSettings == false)
	{
		strcat(ini_name, "\\");
		strcat(ini_name, LauncherSettings::ini_mod_custom);

		dictionary *ini = iniparser_load(ini_name);

		// If settings.ini exists
		if(ini != NULL)
		{
			iniparser_dump(ini, stderr);
			char *flags = strdup(iniparser_getstr(ini, "settings:flags"));
			// Get settings from there
			iniparser_freedict(ini);
			return flags;
		}
	}

	strcat(ini_name, "\\");
	strcat(ini_name, LauncherSettings::ini_mod_default);

	dictionary *ini = iniparser_load(ini_name);

	// If settings.ini doesn't exist
	if(ini == NULL)
	{	
		return NULL;
	}

	iniparser_dump(ini, stderr);
	char *flags = strdup(iniparser_getstr(ini, "settings:flags"));
	iniparser_freedict(ini);

*/
	// Otherwise get from mod.ini
	return "";
}

// This may be called when no mod is selected, in which case mod_selected is set to false
void CTabMOD::SetMOD(char *absolute_path) 
{
	strcpy(m_absolute_text,absolute_path);
	
	// Update the mod name 
	char *relative_path = absolute_path + strlen(LauncherSettings::get_exe_pathonly()) + 1;

	char filename[MAX_PATH];
	strcpy(filename, LauncherSettings::get_active_mod());
	mod_selected = *filename != 0;

	// Set mod names
	SetModName(relative_path, ini_text[INI_MOD_NAME]);

	// Process results - image
	if(ini_text[INI_IMAGE_NAME])
	{
		strcpy(filename, absolute_path);
	   	strcat(filename, "\\");
		strcat(filename, ini_text[INI_IMAGE_NAME]);
		SetMODImage(filename);
	}
	else
	{
		SetMODImage(NULL);
	}

	// Process results - test
	if(ini_text[INI_MOD_TEXT])
	{
		GetDlgItem(IDC_MOD_TEXT)->SetWindowText(ini_text[INI_MOD_TEXT]);
	}
	else
	{
		GetDlgItem(IDC_MOD_TEXT)->SetWindowText("");
	}

	// Process results - buttons
	GetDlgItem(IDC_MOD_WEBSITE)->EnableWindow(ini_text[INI_URL_WEBSITE] != NULL); 
	GetDlgItem(IDC_MOD_FORUM)->EnableWindow(ini_text[INI_URL_FORUM] != NULL); 
}


bool CTabMOD::parse_ini_file(char * ini_name)
{
	dictionary *ini = iniparser_load(ini_name);
	char *p;

	for(int i = 0; i < INI_MAX; i++)
	{
		if(ini_text[i] != NULL)
		{
			free(ini_text[i]);
			ini_text[i] = NULL;
		}
	} 

	if (ini == NULL) 
	{
		return false;
	}

	iniparser_dump(ini, stderr);
	
	ini_text[INI_MOD_NAME]	  = strdup(iniparser_getstr(ini, "launcher:modname"));
	ini_text[INI_IMAGE_NAME]  = strdup(iniparser_getstr(ini, "launcher:image255x112"));
	ini_text[INI_MOD_TEXT]	  =	strdup(iniparser_getstr(ini, "launcher:infotext"));
	ini_text[INI_URL_WEBSITE] = strdup(iniparser_getstr(ini, "launcher:website"));
	ini_text[INI_URL_FORUM]	  =	strdup(iniparser_getstr(ini, "launcher:forum"));
	ini_text[INI_MOD_PRI]	  =	strdup(iniparser_getstr(ini, "multimod:primarylist"));
	
	if (p=iniparser_getstr(ini, "multimod:secondarylist"))
	{
		ini_text[INI_MOD_SEC]  = strdup(p);
	}
	else if (p=iniparser_getstr(ini, "multimod:secondrylist"))
	{
		ini_text[INI_MOD_SEC]  = strdup(p);
	}
	else
	{
		ini_text[INI_MOD_SEC] = NULL;
	}
	
	CheckModSetting(ini_text[INI_MOD_PRI]);
	CheckModSetting(ini_text[INI_MOD_SEC]);

	iniparser_freedict(ini);
	return true;
}

void CTabMOD::CheckModSetting(char *mod_string)
{
	char *current = (mod_string != NULL) ? strchr(mod_string, '-') : NULL;

	while(current != NULL)
	{
		// first, see if this is an embedded dash
		bool embedded_dash = true;
		char *prev = current--;
		if (current == mod_string)	// the first character
			embedded_dash = false;
		else if (*prev == ' ')		// preceded by space
			embedded_dash = false;
		else if (*prev == '\t')		// preceded by tab
			embedded_dash = false;
		else if (*prev == '\n')		// preceded by newline
			embedded_dash = false;

		// if it's a proper start to a command-line option, then check the next few characters
		if(!embedded_dash)
		{
			if(current[1] != 'm' ||
			   current[2] != 'o' ||
			   current[3] != 'd' ||
			   current[4] != ' ')
			{
				MessageBox("Invalid mod.ini multiple mod parameter; only -mod calls are allowed");
				exit(0);
			}
		}

		// continue searching
		current++;
		current = strchr(current, '-');
	}
}

void CTabMOD::SetMODImage(char *path)
{
 	if(MOD_bitmap != NULL)
	{
		::DeleteObject (MOD_bitmap);
		MOD_bitmap = NULL;
	}

	if(path != NULL)
	{
		MOD_bitmap = LoadBitmap(path);

		// Check image size
		BITMAP bm;
		::GetObject (MOD_bitmap, sizeof (bm), & bm);
		if(bm.bmWidth > 255 || bm.bmHeight > 112 )
		{
			MessageBox("Please make image 255x112");
		}
	}

	m_mod_image.SetBitmap(MOD_bitmap);

	CLauncherDlg::Redraw();
}

HBITMAP CTabMOD::LoadBitmap(char * path)
{
    HBITMAP _hBitmap = (HBITMAP) ::LoadImage (0, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (_hBitmap == 0)
	{
		return NULL;
    }

	return _hBitmap;
}

void CTabMOD::GetActiveModName(char *result)
{
	strcpy(result, m_active_mod_name);
}
 
void CTabMOD::GetModCommandLine(char *result)
{
 	char active_mod[MAX_PATH];
	GetActiveModName(active_mod);

	if(strlen(active_mod) == 0)
	{
		*result = '\0';
		return;
	}

	bool need_comma = false;
	strcpy(result, " -mod "); 

	if(ini_text[INI_MOD_PRI] && strlen(ini_text[INI_MOD_PRI]) > 0)
	{
		if (ini_text[INI_MOD_PRI][0] == ',') {
			strcat(result, ini_text[INI_MOD_PRI]+1);
		} else {
			strcat(result, ini_text[INI_MOD_PRI]);
		}
		strcat(result, ",");
	}

	{
		strcat(result, active_mod); 
	}

	if(ini_text[INI_MOD_SEC] && strlen(ini_text[INI_MOD_SEC]) > 0)
	{
		strcat(result, ",");
		if (ini_text[INI_MOD_SEC][0] == ',') {
			strcat(result, ini_text[INI_MOD_SEC]+1);
		} else {
			strcat(result, ini_text[INI_MOD_SEC]);
		}
	}


}

void CTabMOD::SetModName(char *path, char *name)
{
	if(name)
	{
		GetDlgItem(IDC_MOD_NAME)->SetWindowText(name);
	}
	else
	{
		GetDlgItem(IDC_MOD_NAME)->SetWindowText(path ? path : "");
	}

	if(path)
	{
		strcpy(m_active_mod_name, path);
	}
	else
	{
		*m_active_mod_name = '\0';
	}

}

void CTabMOD::OnModNone() 
{
	SetModName(NULL,NULL);
	m_mod_image.SetBitmap(NULL);
	GetDlgItem(IDC_MOD_WEBSITE)->EnableWindow(FALSE); 
	GetDlgItem(IDC_MOD_FORUM)->EnableWindow(FALSE);
	GetDlgItem(IDC_MOD_TEXT)->SetWindowText("");
	LauncherSettings::set_active_mod("");

	CLauncherDlg::Redraw();

	mod_selected = false;
}

void CTabMOD::OnModWebsite() 
{
	open_web_page(ini_text[INI_URL_WEBSITE]);
}

void CTabMOD::OnModForum() 
{
	open_web_page(ini_text[INI_URL_FORUM]);
}

void CTabMOD::OnApply(int flags)
{
	/*
	char flag_str[] = "Test";

	if(mod_selected == false)
	{
		return;
	}


	MessageBox(LauncherSettings::ini_mod_custom);
	FILE *fp = ini_open_for_write(LauncherSettings::ini_mod_custom, true, NULL);

	if(fp)
	{
		ini_write_data(fp, "game_flags", flag_str);
		ini_close(fp);
		fp = NULL;
	}
	*/
}

void CTabMOD::LoadSettings(int flags)
{
	GetSettings();
}

void CTabMOD::SaveSettings()
{
	LauncherSettings::set_active_mod(active_mod);
}