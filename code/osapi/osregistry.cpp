/*
* Copyright (C) Volition, Inc. 1999.  All rights reserved.
*
* All source code herein is the property of Volition, Inc. You may not sell
* or otherwise commercially exploit the source or things you created based on the
* source.
*
*/

#include <string.h>
#include <windows.h>

#include <Shlobj.h>
#include <Sddl.h>

#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"
#include "cmdline/cmdline.h"

// ------------------------------------------------------------------------------------------------------------
// REGISTRY DEFINES/VARS
//

static char	szCompanyName[128];
static char	szAppName[128];
static char	szAppVersion[128];

const char *Osreg_company_name = "Volition";
const char *Osreg_class_name = "FreeSpace2Class";

// RT Lets make all versions use the same registry location
// If we don't the launcher either needs to handle somehow telling what release type a 
// FS2 exe is or it won't work. Its far similar to just use one default location.
// The Launcher will set up everything needed
const char *Osreg_app_name = "FreeSpace2";
const char *Osreg_title = "FreeSpace 2";

const char *Osreg_pref_dir = NULL;

int Os_reg_inited = 0;

// For string config functions
static char tmp_string_data[1024];

// This code is needed for compatibility with the old windows registry

static bool userSIDValid = false;
static SCP_string userSID;

bool get_user_sid(SCP_string& outStr)
{
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == FALSE)
	{
		mprintf(("Failed to get process token! Error Code: %d", GetLastError()));

		return false;
	}

	DWORD dwBufferSize;
	GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize);

	PTOKEN_USER ptkUser = (PTOKEN_USER) new byte[dwBufferSize];

	if (GetTokenInformation(hToken, TokenUser, ptkUser, dwBufferSize, &dwBufferSize))
	{
		CloseHandle(hToken);
	}

	if (IsValidSid(ptkUser->User.Sid) == FALSE)
	{
		mprintf(("Invalid SID structure detected!"));

		delete[] ptkUser;
		return false;
	}

	LPTSTR sidName = NULL;
	if (ConvertSidToStringSid(ptkUser->User.Sid, &sidName) == 0)
	{
		mprintf(("Failed to convert SID structure to string! Error Code: %d", GetLastError()));

		delete[] ptkUser;
		return false;
	}

	outStr.assign(sidName);

	LocalFree(sidName);
	delete[](byte*) ptkUser;

	return true;
}

bool needsWOW64()
{
#ifdef _WIN64
	// 64-bit application always use the Wow6432Node
	return true;
#else
	BOOL bIsWow64 = FALSE;
	if (!IsWow64Process(GetCurrentProcess(), &bIsWow64))
	{
		mprintf(("Failed to determine if we run under Wow64, registry configuration may fail!"));
		return false;
	}

	return bIsWow64 == TRUE;
#endif
}

HKEY get_registry_keyname(char* out_keyname, const char* section)
{
	if (!Cmdline_alternate_registry_path) {
		// Use the original registry path, sometimes breaks for no reason which can be fixed by the code below
		if (section) {
			sprintf(out_keyname, "Software\\%s\\%s\\%s", szCompanyName, szAppName, section);
		}
		else {
			sprintf(out_keyname, "Software\\%s\\%s", szCompanyName, szAppName);
		}
		return HKEY_LOCAL_MACHINE;
	}

	// Every compiler from Visual Studio 2008 onward should have support for UAC
#if _MSC_VER >= 1400
	if (userSIDValid)
	{
		if (needsWOW64())
		{
			if (section) {
				sprintf(out_keyname, "%s_Classes\\VirtualStore\\Machine\\Software\\Wow6432Node\\%s\\%s\\%s", userSID.c_str(), szCompanyName, szAppName, section);
			}
			else {
				sprintf(out_keyname, "%s_Classes\\VirtualStore\\Machine\\Software\\Wow6432Node\\%s\\%s", userSID.c_str(), szCompanyName, szAppName);
			}
		}
		else
		{
			if (section) {
				sprintf(out_keyname, "%s_Classes\\VirtualStore\\Machine\\Software\\%s\\%s\\%s", userSID.c_str(), szCompanyName, szAppName, section);
			}
			else {
				sprintf(out_keyname, "%s_Classes\\VirtualStore\\Machine\\Software\\%s\\%s", userSID.c_str(), szCompanyName, szAppName);
			}
		}

		return HKEY_USERS;
	}
	else
	{
		// This will probably fail
		if (section) {
			sprintf(out_keyname, "Software\\%s\\%s\\%s", szCompanyName, szAppName, section);
		}
		else {
			sprintf(out_keyname, "Software\\%s\\%s", szCompanyName, szAppName);
		}

		return HKEY_LOCAL_MACHINE;
	}
#else
	if (section) {
		sprintf(out_keyname, "Software\\%s\\%s\\%s", szCompanyName, szAppName, section);
	}
	else {
		sprintf(out_keyname, "Software\\%s\\%s", szCompanyName, szAppName);
	}

	return HKEY_LOCAL_MACHINE;
#endif
}

bool registry_has_value(const char *section, const char *name)
{
	HKEY hKey = NULL;
	char keyname[1024];
	LONG lResult;
	bool retVal = false;

	if (!Os_reg_inited) {
		return NULL;
	}

	HKEY useHKey = get_registry_keyname(keyname, section);

	lResult = RegOpenKeyEx(useHKey,			// Where it is
		keyname,			// name of key
		NULL,				// DWORD reserved
		KEY_QUERY_VALUE,	// Allows all changes
		&hKey);			// Location to store key

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	if (!name) {
		goto Cleanup;
	}

	lResult = RegQueryValueEx(hKey,			// Handle to key
		name,			// The values name
		NULL,			// DWORD reserved
		NULL,			// What kind it is
		NULL,			// value to set
		NULL);			// How many bytes to set

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	retVal = true;

Cleanup:
	if (hKey)
		RegCloseKey(hKey);

	return retVal;
}

void os_config_write_string(const char *section, const char *name, const char *value)
{
	HKEY hKey = NULL;
	DWORD dwDisposition;
	char keyname[1024];
	LONG lResult;

	if (!Os_reg_inited) {
		return;
	}

	HKEY useHKey = get_registry_keyname(keyname, section);

	lResult = RegCreateKeyEx(useHKey,						// Where to add it
		keyname,					// name of key
		NULL,						// DWORD reserved
		"",							// Object class
		REG_OPTION_NON_VOLATILE,	// Save to disk
		KEY_ALL_ACCESS,				// Allows all changes
		NULL,						// Default security attributes
		&hKey,						// Location to store key
		&dwDisposition);			// Location to store status of key

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	if (!name) {
		goto Cleanup;
	}

	lResult = RegSetValueEx(hKey,					// Handle to key
		name,					// The values name
		NULL,					// DWORD reserved
		REG_SZ,					// null terminated string
		(CONST BYTE *)value,	// value to set
		strlen(value) + 1);	// How many bytes to set

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}


Cleanup:
	if (hKey)
		RegCloseKey(hKey);
}

void os_config_write_uint(const char *section, const char *name, uint value)
{
	HKEY hKey = NULL;
	DWORD dwDisposition;
	char keyname[1024];
	LONG lResult;

	if (!Os_reg_inited) {
		return;
	}

	HKEY useHKey = get_registry_keyname(keyname, section);

	lResult = RegCreateKeyEx(useHKey,						// Where to add it
		keyname,					// name of key
		NULL,						// DWORD reserved
		"",							// Object class
		REG_OPTION_NON_VOLATILE,	// Save to disk
		KEY_ALL_ACCESS,				// Allows all changes
		NULL,						// Default security attributes
		&hKey,						// Location to store key
		&dwDisposition);			// Location to store status of key

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	if (!name) {
		goto Cleanup;
	}

	lResult = RegSetValueEx(hKey,					// Handle to key
		name,					// The values name
		NULL,					// DWORD reserved
		REG_DWORD,				// null terminated string
		(CONST BYTE *)&value,	// value to set
		4);					// How many bytes to set

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

Cleanup:
	if (hKey)
		RegCloseKey(hKey);

}

// Reads a string from the INI file.  If default is passed,
// and the string isn't found, returns ptr to default otherwise
// returns NULL;    Copy the return value somewhere before
// calling os_read_string again, because it might reuse the
// same buffer.
const char * os_config_read_string(const char *section, const char *name, const char *default_value)
{
	HKEY hKey = NULL;
	DWORD dwType, dwLen;
	char keyname[1024];
	LONG lResult;

	if (!Os_reg_inited) {
		return NULL;
	}

	HKEY useHKey = get_registry_keyname(keyname, section);

	lResult = RegOpenKeyEx(useHKey,			// Where it is
		keyname,			// name of key
		NULL,				// DWORD reserved
		KEY_QUERY_VALUE,	// Allows all changes
		&hKey);			// Location to store key

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	if (!name) {
		goto Cleanup;
	}

	dwLen = 1024;
	lResult = RegQueryValueEx(hKey,									// Handle to key
		name,									// The values name
		NULL,									// DWORD reserved
		&dwType,								// What kind it is
		(ubyte *)&tmp_string_data,				// value to set
		&dwLen);								// How many bytes to set

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	default_value = tmp_string_data;

Cleanup:
	if (hKey)
		RegCloseKey(hKey);

	return default_value;
}

// Reads a string from the INI file.  Default_value must 
// be passed, and if 'name' isn't found, then returns default_value
uint os_config_read_uint(const char *section, const char *name, uint default_value)
{
	HKEY hKey = NULL;
	DWORD dwType, dwLen;
	char keyname[1024];
	LONG lResult;
	uint tmp_val;

	if (!Os_reg_inited) {
		return default_value;
	}

	HKEY useHKey = get_registry_keyname(keyname, section);

	lResult = RegOpenKeyEx(useHKey,			// Where it is
		keyname,			// name of key
		NULL,				// DWORD reserved
		KEY_QUERY_VALUE,	// Allows all changes
		&hKey);			// Location to store key

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	if (!name) {
		goto Cleanup;
	}

	dwLen = 4;
	lResult = RegQueryValueEx(hKey,				// Handle to key
		name,				// The values name
		NULL,				// DWORD reserved
		&dwType,			// What kind it is
		(ubyte *)&tmp_val,	// value to set
		&dwLen);			// How many bytes to set

	if (lResult != ERROR_SUCCESS) {
		goto Cleanup;
	}

	default_value = tmp_val;

Cleanup:
	if (hKey)
		RegCloseKey(hKey);

	return default_value;
}


// initialize the registry. setup default keys to use
void os_init_registry_stuff(const char *company, const char *app, const char *version)
{
	if (company) {
		strcpy_s(szCompanyName, company);
	}
	else {
		strcpy_s(szCompanyName, Osreg_company_name);
	}

	if (app) {
		strcpy_s(szAppName, app);
	}
	else {
		strcpy_s(szAppName, Osreg_app_name);
	}

	if (version) {
		strcpy_s(szAppVersion, version);
	}
	else {
		strcpy_s(szAppVersion, "1.0");
	}

	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versionInfo);

	// Windows Vista is 6.0 which is the first version requiring this
	if (versionInfo.dwMajorVersion >= 6)
	{
		userSIDValid = get_user_sid(userSID);
	}

	Os_reg_inited = 1;
}
