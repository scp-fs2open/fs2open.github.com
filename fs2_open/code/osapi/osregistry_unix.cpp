/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef WIN32	// Goober5000

#include <string.h>
#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"
#include "cfile/cfile.h"
#include "osapi/osapi.h"
#undef malloc
#undef free
#undef strdup
//#undef realloc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ------------------------------------------------------------------------------------------------------------
// REGISTRY DEFINES/VARS
//

char *Osreg_company_name = "Volition";
char *Osreg_class_name = "Freespace2Class";

#if defined(FS2_DEMO)
	char *Osreg_app_name = "FreeSpace2Demo";
	char *Osreg_title = "Freespace 2 Demo";
#ifdef __APPLE__
	char *Osreg_user_dir = "Library/FS2_Open Demo";
#else
	char *Osreg_user_dir = ".freespace2_demo";
#endif // __APPLE__
	#define PROFILE_NAME "FreeSpace2Demo.ini"

#elif defined(OEM_BUILD)
	char *Osreg_app_name = "FreeSpace2OEM";
	char *Osreg_title = "Freespace 2 OEM";
	#define PROFILE_NAME "FreeSpace2OEM.ini"

#else
	char *Osreg_app_name = "FreeSpace2";
	char *Osreg_title = "Freespace 2";
#ifdef __APPLE__
	char *Osreg_user_dir = "Library/FS2_Open";
#else
	char *Osreg_user_dir = ".fs2_open";
#endif // __APPLE__
	#define PROFILE_NAME "fs2_open.ini"
#endif

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


// ------------------------------------------------------------------------------------------------------------
// REGISTRY FUNCTIONS
//

static char *read_line_from_file(CFILE *fp)
{
	char *buf, *buf_start;
	int buflen, len, eol;
	
	buflen = 80;
	buf = (char *)malloc(buflen);
	buf_start = buf;
	eol = 0;
	
	do {
		if (buf == NULL) {
			return NULL;
		}
		
		if (cfgets(buf_start, 80, fp) == NULL) {
			if (buf_start == buf) {
				free(buf);
				return NULL;
			} else {
				*buf_start = 0;
				return buf;
			}
		}
		
		len = strlen(buf_start);
		
		if (buf_start[len-1] == '\n') {
			buf_start[len-1] = 0;
			eol = 1;
		} else {
			buflen += 80;
			
			buf = (char *)realloc(buf, buflen);
			
			/* be sure to skip over the proper amount of nulls */
			buf_start = buf+(buflen-80)-(buflen/80)+1;
		}
	} while (!eol);
	
	return buf;
}

static char *trim_string(char *str)
{
	char *ptr;
	int len;
	
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
	len = strlen(str);
	if (len > 0) {
		ptr += len-1;
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

static Profile *profile_read(char *file)
{
	char fullname[MAX_PATH_LEN];
	snprintf(fullname, MAX_PATH_LEN, "%s%s%s%s%s", detect_home(), DIR_SEPARATOR_STR, Osreg_user_dir, DIR_SEPARATOR_STR, file);
	CFILE *fp = cfopen(fullname, "rt");
	if (fp == NULL)
		return NULL;

	Profile *profile = (Profile *)malloc(sizeof(Profile));
	profile->sections = NULL;
	
	Section **sp_ptr = &(profile->sections);
	Section *sp = NULL;

	KeyValue **kvp_ptr = NULL;
		
	char *str;
	while ((str = read_line_from_file(fp)) != NULL) {
		char *ptr = trim_string(str);
		
		if (*ptr == '[') {
			ptr++;
			
			char *pend = strchr(ptr, ']');
			if (pend != NULL) {
				// if (pend[1]) { /* trailing garbage! */ }
				
				*pend = 0;				
				
				if (*ptr) {
					sp = (Section *)malloc(sizeof(Section));
					sp->next = NULL;
				
					sp->name = strdup(ptr);
					sp->pairs = NULL;
					
					*sp_ptr = sp;
					sp_ptr = &(sp->next);
					
					kvp_ptr = &(sp->pairs);
				} // else { /* null name! */ }
			} // else { /* incomplete section name! */ }
		} else {
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
						KeyValue *kvp = (KeyValue *)malloc(sizeof(KeyValue));
						
						kvp->key = strdup(key);
						kvp->value = strdup(value);
						
						kvp->next = NULL;
						
						*kvp_ptr = kvp;
						kvp_ptr = &(kvp->next);
					} // else { /* key/value with no section! */
				} // else { /* malformed key/value entry! */ }
			} // else it's just a comment or empty string
		}
				
		free(str);
	}
	
	cfclose(fp);

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
			
			free(kvp->key);
			free(kvp->value);
			
			kvp = kvp->next;
			free(kvt);
		}
		
		free(sp->name);
		
		sp = sp->next;
		free(st);
	}
	
	free(profile);
}

static Profile *profile_update(Profile *profile, char *section, char *key, char *value)
{
	if (profile == NULL) {
		profile = (Profile *)malloc(sizeof(Profile));
		
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
					free(kvp->value);
					
					if (value == NULL) {
						*kvp_ptr = kvp->next;
						
						free(kvp->key);
						free(kvp);
					} else {
						kvp->value = strdup(value);
					}
					
					/* all done */
					return profile;
				}
				
				kvp_ptr = &(kvp->next);
				kvp = kvp->next;
			}
			
			if (value != NULL) {
				/* key not found */
				kvp = (KeyValue *)malloc(sizeof(KeyValue));
				kvp->next = NULL;
				kvp->key = strdup(key);
				kvp->value = strdup(value);
			}
					
			*kvp_ptr = kvp;
			
			/* all done */
			return profile;
		}
		
		sp_ptr = &(sp->next);
		sp = sp->next;
	}
	
	/* section not found */
	sp = (Section *)malloc(sizeof(Section));
	sp->next = NULL;
	sp->name = strdup(section);
	
	kvp = (KeyValue *)malloc(sizeof(KeyValue));
	kvp->next = NULL;
	kvp->key = strdup(key);
	kvp->value = strdup(value);
	
	sp->pairs = kvp;
	
	*sp_ptr = sp;
	
	return profile;
}

static char *profile_get_value(Profile *profile, char *section, char *key)
{
	if (profile == NULL)
		return NULL;
	
	Section *sp = profile->sections;
	while (sp != NULL) {
		if (strcmp(section, sp->name) == 0) {
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

static void profile_save(Profile *profile, char *file)
{
	CFILE *fp;
	
	char tmp[MAX_PATH] = "";
	char tmp2[MAX_PATH] = "";
	
	if (profile == NULL)
		return;

	char fullname[MAX_PATH_LEN];
	snprintf(fullname, MAX_PATH_LEN, "%s%s%s%s%s", detect_home(), DIR_SEPARATOR_STR, Osreg_user_dir, DIR_SEPARATOR_STR, file);
	fp = cfopen(fullname, "wt");
	if (fp == NULL)
		return;
	
	Section *sp = profile->sections;
	while (sp != NULL) {
		sprintf(tmp, NOX("[%s]\n"), sp->name);
		cfputs(tmp, fp);
		
		KeyValue *kvp = sp->pairs;
		while (kvp != NULL) {
			sprintf(tmp2, NOX("%s=%s\n"), kvp->key, kvp->value);
			cfputs(tmp2, fp);
			kvp = kvp->next;
		}
		
		cfwrite_char('\n', fp);
		
		sp = sp->next;
	}
	
	cfclose(fp);
}

// os registry functions -------------------------------------------------------------

static char			szCompanyName[128];
static char			szAppName[128];
static char			szAppVersion[128];

int Os_reg_inited = 0;

// initialize the registry. setup default keys to use
void os_init_registry_stuff(char *company, char *app, char *version)
{
	if(company){
		strcpy( szCompanyName, company );
	} else {
		strcpy( szCompanyName, Osreg_company_name);
	}

	if(app){
		strcpy( szAppName, app );
	} else {
		strcpy( szAppName, Osreg_app_name);
	}

	if(version){
		strcpy( szAppVersion, version);
	} else {
		strcpy( szAppVersion, "1.0");
	}

	Os_reg_inited = 1;
}

static char tmp_string_data[1024];

char *os_config_read_string(char *section, char *name, char *default_value)
{
	nprintf(( "Warning", "REGISTRY: %s in %s at line %d, thread %d - section = \"%s\", name = \"%s\", default value: \"%s\"\n", 
			__FUNCTION__, __FILE__, __LINE__, getpid(), section, name, default_value ));

	Profile *p = profile_read(PROFILE_NAME);

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

unsigned int os_config_read_uint(char *section, char *name, unsigned int default_value)
{
	Profile *p = profile_read(PROFILE_NAME);
	
	if (section == NULL)
		section = DEFAULT_SECTION;
		
	char *ptr = profile_get_value(p, section, name);
	if (ptr != NULL) {
		default_value = atoi(ptr);
	}
	
	profile_free(p);
	
	return default_value;
}

void os_config_write_string(char *section, char *name, char *value)
{
	Profile *p = profile_read(PROFILE_NAME);
	
	if (section == NULL)
		section = DEFAULT_SECTION;
		
	p = profile_update(p, section, name, value);
	profile_save(p, PROFILE_NAME);
	profile_free(p);	
}

void os_config_write_uint(char *section, char *name, unsigned int value)
{
	static char buf[21];
	
	snprintf(buf, 20, "%u", value);
	
	Profile *p = profile_read(PROFILE_NAME);
	
	if (section == NULL)
		section = DEFAULT_SECTION;
	
	p = profile_update(p, section, name, buf);
	profile_save(p, PROFILE_NAME);
	profile_free(p);
}

#endif		// Goober5000 - #ifndef WIN32
