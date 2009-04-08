#include "stdafx.h"
#include "win32func.h"

void open_web_page(char *web_url)
{
	ShellExecute(NULL, "open", web_url, NULL, NULL, SW_SHOWDEFAULT);
//	WinExec(web_url,SW_SHOW);
}

bool browse_for_open_file(HWND hwnd, char *filename, char *ext, char *title)
{
	OPENFILENAME details;
	memset(&details, 0, sizeof(OPENFILENAME));
	details.lStructSize       = sizeof(OPENFILENAME); 
	details.hwndOwner         = hwnd; 
	details.lpstrFilter       = 0;//ext; 
	details.lpstrInitialDir   = "";
	details.lpstrTitle        = title; 
	details.Flags			  = OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST; 

	details.lpstrDefExt		  = details.lpstrFilter;

	details.lpstrFile         = filename;
	details.nMaxFile          = MAX_PATH;

	if(GetOpenFileName(&details) == FALSE)
	{
		return false;
	}

	return true;
}

bool browse_for_save_file(HWND hwnd, char *filename, char *ext, char *title)
{
	OPENFILENAME details;
	memset(&details, 0, sizeof(OPENFILENAME));
	details.lStructSize       = sizeof(OPENFILENAME); 
	details.hwndOwner         = hwnd; 
	details.lpstrFilter       = ext; 
	details.lpstrInitialDir   = "";
	details.lpstrTitle        = title; 
	details.Flags			  = OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST; 

	details.lpstrDefExt		  = details.lpstrFilter;

	details.lpstrFile         = filename;
	details.nMaxFile          = MAX_PATH;

	if(GetSaveFileName(&details) == FALSE)
	{
		return false;
	}

	return true;
}

/**
 * Opens a standard save dailog box that allows the user to browse for a save location and automatically open the file
 *
 * @param HWND hwnd		 - Handle to parent window 
 * @param char *filename - pointer to buffer that hols the default file name and will hold the chosen file name
 * @param char *ext		 - Extension of file to save, ie "txt", "exe"
 * @param char *title	 - Window title
 *
 * Recommend filename is of size MAX_PATH
 *
 * @return FILE * - Pointer to open file or NULL if user cancelled or file could not be opened 
 */
FILE *browse_for_and_open_save_file(HWND hwnd, char *filename, char *ext, char *title)
{
	if(strlen(filename))
	{
		strcat(filename, ".");
		strcat(filename, ext);
	}

	if(browse_for_save_file(hwnd, filename, ext, title) == false)
	{
		return NULL;
	}


	return fopen(filename,"w");
}

FILE *browse_for_and_open_open_file(HWND hwnd, char *filename, char *ext, char *title)
{
	if(strlen(filename))
	{
		strcat(filename, ".");
		strcat(filename, ext);
	}

	if(browse_for_open_file(hwnd, filename, ext, title) == false)
	{
		return NULL;
	}


	return fopen(filename,"w");
}

bool browse_for_dir(HWND hwnd, char *root, char *result, char *title)
{
	LPITEMIDLIST  pidl;
	LPSHELLFOLDER pDesktopFolder;
	OLECHAR       olePath[MAX_PATH];
	ULONG         chEaten;
	HRESULT       hr;

	*result = '\0';
	// Get the path to the file we need to convert.
	GetCurrentDirectory(MAX_PATH, root);

	// Get a pointer to the Desktop's IShellFolder interface.
	if (FAILED(SHGetDesktopFolder(&pDesktopFolder)))
	{
		return true;
	}

	// IShellFolder::ParseDisplayName requires the file name be in Unicode.
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, root, -1, olePath, MAX_PATH);

	// Convert the path to an ITEMIDLIST.
	hr = pDesktopFolder->ParseDisplayName(hwnd, NULL, olePath, &chEaten, &pidl, NULL);
	if (FAILED(hr))
	{
		return true;
	}

	// pidl now contains a pointer to an ITEMIDLIST 

	// Browse for the file
	BROWSEINFO browse_info;

    browse_info.hwndOwner = hwnd; 
	browse_info.pidlRoot  = pidl; 
    browse_info.pszDisplayName = NULL;
    browse_info.lpszTitle = title; 
    browse_info.ulFlags = 0; 
    browse_info.lpfn = NULL; 
    browse_info.lParam = 0; 
    browse_info.iImage = 0; 

	LPITEMIDLIST dirs = SHBrowseForFolder(&browse_info); 

	//release the desktop folder object
    pDesktopFolder->Release();

	if(dirs == NULL) 
		return false;

	SHGetPathFromIDList(dirs, result); 

	return true;
}

/**
 * Runs an exe as a seperate process
 *
 * @param char *app_name  - Name of app to run
 * @param char *app_path  - Path of app to run
 * @param char *comm_line - Command line (optional)
 *
 * @return bool - states if exe ran OK
 */
bool run_file(char *app_name, char *app_path, char *comm_line, bool wait)
{
	PROCESS_INFORMATION process_info;
	STARTUPINFO startup_info;

	memset(&startup_info, 0, sizeof(STARTUPINFO));
	startup_info.cb = sizeof(STARTUPINFO); 

	SetErrorMode(SEM_NOOPENFILEERRORBOX);

	BOOL result = 
		CreateProcess(
			app_name,
			comm_line,	   
			NULL, NULL, FALSE, 
			CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE,	// flags
			NULL,
			app_path,	
			&startup_info,
			&process_info);

	if(result == FALSE)
	{
		return false;
	}

	if(wait)
	{
		DWORD exit_code;

		do 
		{
			// Sleep so we dont query madly and slow everything down
			Sleep(20);
			GetExitCodeProcess(process_info.hProcess, &exit_code);
		} while (exit_code == STILL_ACTIVE);
	}

	return result != FALSE;
}

/**
 * Open a registry dir ready for enumeration
 *
 * @param const char *reg_path - Path to open
 * @return HKEY	- HKEY for that path or NULL if failed
 */
HKEY reg_open_dir(const char *reg_path)
{
	if (reg_path == NULL)
		return NULL;

	// Try to open the correct registry dir
	HKEY hkey;

	if ( RegOpenKeyEx(HKEY_CURRENT_USER, reg_path, 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS )
		return NULL;

	return hkey;

}

HKEY reg_create_dir(const char *reg_path)
{
	HKEY new_key;

	if (reg_path == NULL)
		return NULL;

	if ( RegCreateKeyEx(HKEY_CURRENT_USER, reg_path, 0,
			NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
			NULL, &new_key, NULL) != ERROR_SUCCESS )
	{
		return NULL;
	}

	return new_key;
}

/**
 * General function for changing registry entries, creates if they dont already exist
 *
 * @param const char *reg_path - Path of key
 * @param const char *keyname  - Name of subkey
 * @param int reg_type		   - HKEY_CLASSES_ROOT, HKEY_CURRENT_CONFIG, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_USERS
 * @param void *data		   - Pointer to the data
 * @param int data_len		   - Length of data in buffer
 *
 * @return bool - states if value set happened OK
 */
bool reg_set_value(const char *reg_path, const char *keyname, int reg_type, void *data, int data_len)
{
	// Try to open the correct registry dir
	HKEY hkey = reg_create_dir(reg_path);

	if (hkey == NULL)
		return false;

	if ( RegSetValueEx(hkey, keyname, 0, reg_type, (BYTE *) data, data_len) != ERROR_SUCCESS ) {
		RegCloseKey(hkey);
		return false;
	}

	RegFlushKey(hkey);
  	RegCloseKey(hkey);

	return true;
}

/**
 * Changes the content of a string registry entry or created the entry if its not already
 *
 * @param const char *reg_path - Path of key
 * @param const char *keyname  - Name of the subkey
 * @param const char *string   - String to insert into registry, ensure that it has a terminator
 *
 * @return bool - states if string set happened OK
 */
bool reg_set_sz(const char *reg_path, const char *keyname, const char *string)
{
	return reg_set_value(
		reg_path,
		keyname, 
		REG_SZ, 
		(void *) string, 
		(strlen(string) + 1) * sizeof(char));
}

/**
 * Changes the value of a value registry entry or created the entry if its not already
 *
 * @param const char *reg_path - Path of key
 * @param const char *keyname  - Name of the subkey
 * @param DWORD value		   - Value to be inserted into the registry
 *
 * @return bool - states if value set happened OK
 */
bool reg_set_dword(const char *reg_path, const char *keyname, DWORD value)
{
	return reg_set_value(reg_path, keyname, REG_DWORD, &value, sizeof(DWORD));
}

/**
 * General function for retrieving registry entries
 *
 * @param const char *reg_path - Path of key
 * @param const char *keyname  - Name of subkey
 * @param int reg_type		   - HKEY_CLASSES_ROOT, HKEY_CURRENT_CONFIG, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_USERS
 * @param void *data		   - Pointer to the data
 * @param int *data_len		   - Pointer to length of data in buffer
 *
 * @return bool - states if value get happened OK
 */
bool reg_get_value(
	const char *reg_path, 
	const char *keyname, 
	unsigned long reg_type, 
	void *data, 
	unsigned long *data_len)
{
	// Try to open the correct registry dir
	HKEY hkey = reg_open_dir(reg_path);

	if(hkey == NULL)
	{
		return false;
	}

	unsigned long check_reg_type;

	if(RegQueryValueEx(hkey, keyname, 0, &check_reg_type, (BYTE *) data, data_len) != ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		return false;
	}

	RegCloseKey(hkey);

	return (reg_type == check_reg_type);
}

/**
 * Retrieves the content of a string registry entry 
 *
 * @param const char *reg_path - Path of key
 * @param const char *keyname  - Name of the subkey
 * @param const char *string   - Buufer to hold string from the registry
 * @param unsigned long length - Size of string buffer provided
 *
 * @return bool - states if string get happened OK
 */
bool reg_get_sz(const char *reg_path, const char *keyname, char *string, unsigned long length)
{
	string[0] = '\0';
	return reg_get_value(
		reg_path,
		keyname, 
		REG_SZ, 
		(void *) string, 
		&length);
}

/**
 * Retrieves the value of a value registry entry
 *
 * @param const char *reg_path - Path of key
 * @param const char *keyname  - Name of the subkey
 * @param DWORD *value		   - Value to be retrieved into the registry
 *
 * @return bool - states if value get happened OK
 */
bool reg_get_dword(const char *reg_path, const char *keyname, DWORD *value)
{
	unsigned long size = sizeof(DWORD);
	return reg_get_value(reg_path, keyname, REG_DWORD, value, &size);
}

char *os_type_string[OS_MAX] = 
{
	"Failed to get OS type",
	"Win NT 3.51",
	"Win NT 4",
	"Win 95",
	"Win 98",
	"Win Me",
	"Win 2000",
	"Win XP",
	"Win Server 2003",
	"Unknown",
};

int os_get_type(char *service_pack)
{
	OSVERSIONINFO version_info;
	version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if(service_pack) *service_pack = '\0';

	if(GetVersionEx(&version_info) == false)
	{
		return OS_FAILED;
	}

	if(service_pack) 
		strcpy(service_pack, version_info.szCSDVersion);

	switch(version_info.dwMajorVersion)
	{
		case 4:
		{
			if(version_info.dwMinorVersion ==  0) 
			{
				return (version_info.dwPlatformId != VER_PLATFORM_WIN32_NT) ? OS_WIN_95 : OS_WIN_NT4;
			}
			if(version_info.dwMinorVersion == 10) return OS_WIN_98;
			if(version_info.dwMinorVersion == 90) return OS_WIN_ME;
			break;
		}

		case 3:
		{
			if(version_info.dwMinorVersion == 51) return OS_WIN_NT3;
			break;
		}					   

		case 5:
		{
			if(version_info.dwMinorVersion == 0) return OS_WIN_2000;
			if(version_info.dwMinorVersion == 1) return OS_WIN_XP;
			if(version_info.dwMinorVersion == 2) return OS_WIN_SERVER_2003;
			break;
		}
	}

	return OS_WIN_UNKNOWN;
}

void os_get_type_text(char *buffer)
{
	char service_pack_string[100] = "";
	strcpy(buffer, os_type_string[os_get_type(service_pack_string)]);

	if(strlen(service_pack_string) > 0)
	{
		strcat(buffer, " ");
		strcat(buffer, service_pack_string);
	}

}

int memory_get_ammount()
{

	MEMORYSTATUS mem_stats;
	GlobalMemoryStatus(&mem_stats);

	return mem_stats.dwTotalPhys;
}