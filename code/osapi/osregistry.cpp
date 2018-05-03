/*
* Copyright (C) Volition, Inc. 1999.  All rights reserved.
*
* All source code herein is the property of Volition, Inc. You may not sell
* or otherwise commercially exploit the source or things you created based on the
* source.
*
*/

#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#ifdef WIN32
#include <windows.h>
// Stupid Microsoft is not able to fix a simple compile warning: https://connect.microsoft.com/VisualStudio/feedback/details/1342304/level-1-compiler-warnings-in-windows-sdk-shipped-with-visual-studio
#pragma warning(push)
#pragma warning(disable: 4091) // ignored on left of '' when no variable is declared
#include <shlobj.h>
#pragma warning(pop)
#include <sddl.h>
#endif

namespace
{
	// ------------------------------------------------------------------------------------------------------------
	// REGISTRY FUNCTIONS
	//

	char szCompanyName[128];
	char szAppName[128];

	int Os_reg_inited = 0;

#ifdef WIN32
	// os registry functions -------------------------------------------------------------

	// This code is needed for compatibility with the old windows registry

	static bool userSIDValid = false;
	static SCP_string userSID;

	bool get_user_sid(SCP_string& outStr)
	{
		HANDLE hToken = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == FALSE)
		{
			mprintf(("Failed to get process token! Error Code: %d", (int)GetLastError()));

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
			mprintf(("Failed to convert SID structure to string! Error Code: %d", (int)GetLastError()));

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
#if IS_64BIT
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
					sprintf(out_keyname, "%s_Classes\\VirtualStore\\MACHINE\\SOFTWARE\\WOW6432Node\\%s\\%s\\%s", userSID.c_str(), szCompanyName, szAppName, section);
				}
				else {
					sprintf(out_keyname, "%s_Classes\\VirtualStore\\MACHINE\\SOFTWARE\\WOW6432Node\\%s\\%s", userSID.c_str(), szCompanyName, szAppName);
				}
			}
			else
			{
				if (section) {
					sprintf(out_keyname, "%s_Classes\\VirtualStore\\MACHINE\\SOFTWARE\\%s\\%s\\%s", userSID.c_str(), szCompanyName, szAppName, section);
				}
				else {
					sprintf(out_keyname, "%s_Classes\\VirtualStore\\MACHINE\\SOFTWARE\\%s\\%s", userSID.c_str(), szCompanyName, szAppName);
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

	void registry_write_string(const char *section, const char *name, const char *value)
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
			0,						// DWORD reserved
			NULL,						// Object class
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
			0,						// DWORD reserved
			REG_SZ,					// null terminated string
			(CONST BYTE *)value,	// value to set
			(DWORD)(strlen(value) + 1));	// How many bytes to set

		if (lResult != ERROR_SUCCESS) {
			goto Cleanup;
		}


	Cleanup:
		if (hKey)
			RegCloseKey(hKey);
	}

	void registry_write_uint(const char *section, const char *name, uint value)
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
			0,							// DWORD reserved
			NULL,						// Object class
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
			0,						// DWORD reserved
			REG_DWORD,				// null terminated string
			(CONST BYTE *)&value,	// value to set
			4);						// How many bytes to set

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
	static char registry_tmp_string_data[1024];
	const char * registry_read_string(const char *section, const char *name, const char *default_value)
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
			0,					// DWORD reserved
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
			(ubyte *)&registry_tmp_string_data,				// value to set
			&dwLen);								// How many bytes to set

		if (lResult != ERROR_SUCCESS) {
			goto Cleanup;
		}

		default_value = registry_tmp_string_data;

	Cleanup:
		if (hKey)
			RegCloseKey(hKey);

		return default_value;
	}

	// Reads a string from the INI file.  Default_value must 
	// be passed, and if 'name' isn't found, then returns default_value
	uint registry_read_uint(const char *section, const char *name, uint default_value)
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
			0,					// DWORD reserved
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
#endif
}

// ------------------------------------------------------------------------------------------------------------
// REGISTRY DEFINES/VARS
//
#ifdef SCP_UNIX
// Initialize path of old pilot files
#ifdef __APPLE__
const char* Osreg_user_dir_legacy = "Library/FS2_Open";
#else
const char* Osreg_user_dir_legacy = ".fs2_open";
#endif // __APPLE__
#endif

const char *Osreg_company_name = "Volition";
const char *Osreg_class_name = "FreeSpace2Class";

const char *Osreg_app_name = "FreeSpace2";
const char *Osreg_title = "FreeSpace 2";

const char *Osreg_config_file_name = "fs2_open.ini";

#define DEFAULT_SECTION "Default"

typedef struct KeyValue
{
	char *key;
	char *value;

	struct KeyValue *next;
} KeyValue;

typedef struct Section
{
	char *name;

	struct KeyValue *pairs;
	struct Section *next;
} Section;

typedef struct Profile
{
	struct Section *sections;
} Profile;

// For string config functions
static char tmp_string_data[1024];

// This code is needed for compatibility with the old windows registry

static char *read_line_from_file(FILE *fp)
{
	char *buf, *buf_start;
	int buflen, eol;

	buflen = 80;
	buf = (char *)vm_malloc(buflen);
	buf_start = buf;
	eol = 0;

	do {
		if (buf == NULL) {
			return NULL;
		}

		if (fgets(buf_start, 80, fp) == NULL) {
			if (buf_start == buf) {
				vm_free(buf);
				return NULL;
			}
			else {
				*buf_start = 0;
				return buf;
			}
		}

		auto len = strlen(buf_start);

		if (buf_start[len - 1] == '\n') {
			buf_start[len - 1] = 0;
			eol = 1;
		}
		else {
			buflen += 80;

			buf = (char *)vm_realloc(buf, buflen);

			/* be sure to skip over the proper amount of nulls */
			buf_start = buf + (buflen - 80) - (buflen / 80) + 1;
		}
	} while (!eol);

	return buf;
}

static char *trim_string(char *str)
{
	char *ptr;

	if (str == NULL)
		return NULL;

	/* kill any comment */
	ptr = strchr(str, ';');
	if (ptr)
		*ptr = 0;
	ptr = strchr(str, '#');
	if (ptr)
		*ptr = 0;

	ptr = str;
	auto len = strlen(str);
	if (len > 0) {
		ptr += len - 1;
	}

	while ((ptr > str) && isspace(*ptr)) {
		ptr--;
	}

	if (*ptr) {
		ptr++;
		*ptr = 0;
	}

	ptr = str;
	while (*ptr && isspace(*ptr)) {
		ptr++;
	}

	return ptr;
}

static Profile *profile_read(const char *file)
{
	FILE *fp = NULL;
	char *str;

	if (os_is_legacy_mode()) {
#ifdef WIN32
		return nullptr; // No config file in legacy mode
#else
		// Try to use the config file at the old location
		char legacy_path[MAX_PATH_LEN];
		snprintf(legacy_path, MAX_PATH_LEN, "%s/%s/%s", getenv("HOME"), Osreg_user_dir_legacy, file);

		fp = fopen(legacy_path, "rt");
#endif
	}
	else {
		fp = fopen(os_get_config_path(file).c_str(), "rt");
	}

	if (fp == NULL)
		return NULL;

	Profile *profile = (Profile *)vm_malloc(sizeof(Profile));
	profile->sections = NULL;

	Section **sp_ptr = &(profile->sections);
	Section *sp = NULL;

	KeyValue **kvp_ptr = NULL;

	while ((str = read_line_from_file(fp)) != NULL) {
		char *ptr = trim_string(str);

		if (*ptr == '[') {
			ptr++;

			char *pend = strchr(ptr, ']');
			if (pend != NULL) {
				// if (pend[1]) { /* trailing garbage! */ }

				*pend = 0;

				if (*ptr) {
					sp = (Section *)vm_malloc(sizeof(Section));
					sp->next = NULL;

					sp->name = vm_strdup(ptr);
					sp->pairs = NULL;

					*sp_ptr = sp;
					sp_ptr = &(sp->next);

					kvp_ptr = &(sp->pairs);
				} // else { /* null name! */ }
			} // else { /* incomplete section name! */ }
		}
		else {
			if (*ptr) {
				char *key = ptr;
				char *value = NULL;

				ptr = strchr(ptr, '=');
				if (ptr != NULL) {
					*ptr = 0;
					ptr++;

					value = ptr;
				} // else { /* random garbage! */ }

				if (key && *key && value /* && *value */) {
					if (sp != NULL) {
						KeyValue *kvp = (KeyValue *)vm_malloc(sizeof(KeyValue));

						kvp->key = vm_strdup(key);
						kvp->value = vm_strdup(value);

						kvp->next = NULL;

						*kvp_ptr = kvp;
						kvp_ptr = &(kvp->next);
					} // else { /* key/value with no section! */
				} // else { /* malformed key/value entry! */ }
			} // else it's just a comment or empty string
		}

		vm_free(str);
	}

	fclose(fp);

	return profile;
}

static void profile_free(Profile *profile)
{
	if (profile == NULL)
		return;

	Section *sp = profile->sections;
	while (sp != NULL) {
		Section *st = sp;
		KeyValue *kvp = sp->pairs;

		while (kvp != NULL) {
			KeyValue *kvt = kvp;

			vm_free(kvp->key);
			vm_free(kvp->value);

			kvp = kvp->next;
			vm_free(kvt);
		}

		vm_free(sp->name);

		sp = sp->next;
		vm_free(st);
	}

	vm_free(profile);
}

static Profile *profile_update(Profile *profile, const char *section, const char *key, const char *value)
{
	if (profile == NULL) {
		profile = (Profile *)vm_malloc(sizeof(Profile));

		profile->sections = NULL;
	}

	KeyValue *kvp;

	Section **sp_ptr = &(profile->sections);
	Section *sp = profile->sections;

	while (sp != NULL) {
		if (strcmp(section, sp->name) == 0) {
			KeyValue **kvp_ptr = &(sp->pairs);
			kvp = sp->pairs;

			while (kvp != NULL) {
				if (strcmp(key, kvp->key) == 0) {
					vm_free(kvp->value);

					if (value == NULL) {
						*kvp_ptr = kvp->next;

						vm_free(kvp->key);
						vm_free(kvp);
					}
					else {
						kvp->value = vm_strdup(value);
					}

					/* all done */
					return profile;
				}

				kvp_ptr = &(kvp->next);
				kvp = kvp->next;
			}

			if (value != NULL) {
				/* key not found */
				kvp = (KeyValue *)vm_malloc(sizeof(KeyValue));
				kvp->next = NULL;
				kvp->key = vm_strdup(key);
				kvp->value = vm_strdup(value);
			}

			*kvp_ptr = kvp;

			/* all done */
			return profile;
		}

		sp_ptr = &(sp->next);
		sp = sp->next;
	}

	/* section not found */
	sp = (Section *)vm_malloc(sizeof(Section));
	sp->next = NULL;
	sp->name = vm_strdup(section);

	kvp = (KeyValue *)vm_malloc(sizeof(KeyValue));
	kvp->next = NULL;
	kvp->key = vm_strdup(key);
	kvp->value = vm_strdup(value);

	sp->pairs = kvp;

	*sp_ptr = sp;

	return profile;
}

static char *profile_get_value(Profile *profile, const char *section, const char *key)
{
	if (profile == NULL)
		return NULL;

	Section *sp = profile->sections;

	while (sp != NULL) {
		if (stricmp(section, sp->name) == 0) {
			KeyValue *kvp = sp->pairs;

			while (kvp != NULL) {
				if (strcmp(key, kvp->key) == 0) {
					return kvp->value;
				}
				kvp = kvp->next;
			}
		}

		sp = sp->next;
	}

	/* not found */
	return NULL;
}

static void profile_save(Profile *profile, const char *file)
{
	FILE *fp = NULL;
	char tmp[MAX_PATH] = "";
	char tmp2[MAX_PATH] = "";

	if (profile == NULL)
		return;

	fp = fopen(os_get_config_path(file).c_str(), "wt");

	if (fp == NULL)
		return;

	Section *sp = profile->sections;

	while (sp != NULL) {
		sprintf(tmp, NOX("[%s]\n"), sp->name);
		fputs(tmp, fp);

		KeyValue *kvp = sp->pairs;
		while (kvp != NULL) {
			sprintf(tmp2, NOX("%s=%s\n"), kvp->key, kvp->value);
			fputs(tmp2, fp);
			kvp = kvp->next;
		}

		fprintf(fp, "\n");

		sp = sp->next;
	}

	fclose(fp);
}

// os registry functions -------------------------------------------------------------

// initialize the registry. setup default keys to use
void os_init_registry_stuff(const char *company, const char *app)
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

#ifdef WIN32
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versionInfo);

	// Windows Vista is 6.0 which is the first version requiring this
	if (versionInfo.dwMajorVersion >= 6)
	{
		userSIDValid = get_user_sid(userSID);
	}
#endif

	Os_reg_inited = 1;
}

const char *os_config_read_string(const char *section, const char *name, const char *default_value)
{
	Profile *p = profile_read(Osreg_config_file_name);

#ifdef WIN32
	if (p == nullptr) {
		// No config file, fall back to registy
		return registry_read_string(section, name, default_value);
	}
#endif
	nprintf(("Registry", "os_config_read_string(): section = \"%s\", name = \"%s\", default value: \"%s\"\n",
		(section) ? section : DEFAULT_SECTION, name, (default_value) ? default_value : NOX("NULL")));

	if (section == NULL)
		section = DEFAULT_SECTION;

	char *ptr = profile_get_value(p, section, name);

	if (ptr != NULL) {
		strncpy(tmp_string_data, ptr, 1023);
		default_value = tmp_string_data;
	}

	profile_free(p);

	return default_value;
}

unsigned int os_config_read_uint(const char *section, const char *name, unsigned int default_value)
{
	Profile *p = profile_read(Osreg_config_file_name);

#ifdef WIN32
	if (p == nullptr) {
		// No config file, fall back to registy
		return registry_read_uint(section, name, default_value);
	}
#endif

	if (section == NULL)
		section = DEFAULT_SECTION;

	char *ptr = profile_get_value(p, section, name);

	if (ptr != NULL) {
		default_value = atoi(ptr);
	}

	profile_free(p);

	return default_value;
}

void os_config_write_string(const char *section, const char *name, const char *value)
{
	Profile *p = profile_read(Osreg_config_file_name);

#ifdef WIN32
	// When there is no config file then it shouldn't be created because that would "hide" all previous settings
	// Instead fall back to writing the settings to the config file
	if (p == nullptr) {
		registry_write_string(section, name, value);
		return;
	}
#endif

	if (section == NULL)
		section = DEFAULT_SECTION;

	p = profile_update(p, section, name, value);
	profile_save(p, Osreg_config_file_name);
	profile_free(p);
}

void os_config_write_uint(const char *section, const char *name, unsigned int value)
{

	Profile *p = profile_read(Osreg_config_file_name);

#ifdef WIN32
	// When there is no config file then it shouldn't be created because that would "hide" all previous settings
	// Instead fall back to writing the settings to the config file
	if (p == nullptr) {
		registry_write_uint(section, name, value);
		return;
	}
#endif

	if (section == NULL)
		section = DEFAULT_SECTION;

	char buf[21];

	snprintf(buf, 20, "%u", value);

	p = profile_update(p, section, name, buf);
	profile_save(p, Osreg_config_file_name);
	profile_free(p);
}

