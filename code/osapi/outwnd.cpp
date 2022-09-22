/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <algorithm>

#ifdef WIN32
#include <direct.h>
#endif

#include "osapi/DebugWindow.h"
#include "osapi/osapi.h"
#include "osapi/outwnd.h"
#include "graphics/2d.h"
#include "freespaceresource.h"
#include "globalincs/systemvars.h"
#include "cfile/cfilesystem.h"
#include "parse/parselo.h"

static const char *FILTERS_ENABLED_BY_DEFAULT[] =
{
	"error",
	"warning",
	"general",
	"scripting"
};

struct outwnd_filter_struct {
	char name[NAME_LENGTH];
	bool enabled;
};

// This technique is necessary to avoid a crash in FRED.  See commit e624f6c1.
static SCP_vector<outwnd_filter_struct>& filter_vector()
{
	static SCP_vector<outwnd_filter_struct> vec;
	return vec;
}

// used for file logging
bool Log_debug_output_to_file = true;

void outwnd_print(const char* id = nullptr, const char* temp = nullptr);

// 0 = .cfg file found, 1 = not found and warning not printed yet, 2 = not found and warning printed
static ubyte Outwnd_no_filter_file = 0;

static ubyte outwnd_filter_loaded = 0;
static bool outwnd_inited         = false;

static FILE* Log_fp                      = nullptr;
static bool Log_close_fp                 = false;
static const char* FreeSpace_logfilename = nullptr;

static std::unique_ptr<osapi::DebugWindow> debugWindow;

void load_filter_info()
{
	FILE* fp;
	char pathname[MAX_PATH_LEN];
	char inbuf[NAME_LENGTH + 4];
	outwnd_filter_struct* filter_ptr;

	// add default-enabled filters
	for (const char *name : FILTERS_ENABLED_BY_DEFAULT) {
		filter_vector().emplace_back();
		filter_ptr = &filter_vector().back();

		strcpy_s(filter_ptr->name, name);
		filter_ptr->enabled = true;
	}

	outwnd_filter_loaded = 1;

	memset(pathname, 0, sizeof(pathname));
	snprintf(pathname, MAX_PATH_LEN, "%s/%s", Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg"));

	fp = fopen(os_get_config_path(pathname).c_str(), "rt");

	if (!fp) {
		Outwnd_no_filter_file = 1;
		return;
	}

	Outwnd_no_filter_file = 0;

	while ( fgets(inbuf, NAME_LENGTH+3, fp) ) {
		bool enabled;

		if (*inbuf == '+')
			enabled = true;
		else if (*inbuf == '-')
			enabled = false;
		else
			continue;	// skip everything else

		auto z = strlen(inbuf) - 1;
		if (inbuf[z] == '\n')
			inbuf[z] = 0;

		Assert( strlen(inbuf+1) < NAME_LENGTH );
		const char *name = &inbuf[1];

		// find it, or create it
		filter_ptr = nullptr;
		for (auto &filter : filter_vector()) {
			if (!stricmp(filter.name, name)) {
				filter_ptr = &filter;
				break;
			}
		}
		if (filter_ptr == nullptr) {
			filter_vector().emplace_back();
			filter_ptr = &filter_vector().back();
			strcpy_s(filter_ptr->name, name);
		}

		// set it as enabled or not, depending on config
		filter_ptr->enabled = enabled;
	}

	if ( ferror(fp) && !feof(fp) )
		nprintf(("Error", "Error reading \"%s\"\n", pathname));

	fclose(fp);
}

void save_filter_info()
{
	FILE* fp;
	char pathname[MAX_PATH_LEN];

	if (!outwnd_filter_loaded)
		return;

	if (Outwnd_no_filter_file)
		return; // No file, don't save

	memset(pathname, 0, sizeof(pathname));
	snprintf(pathname, MAX_PATH_LEN, "%s/%s", Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg"));

	fp = fopen(os_get_config_path(pathname).c_str(), "wt");

	if (fp) {
		for (auto& i : filter_vector()) {
			fprintf(fp, "%c%s\n", i.enabled ? '+' : '-', i.name);
		}

		fclose(fp);
	}
}

void outwnd_printf2(const char *format, ...)
{
	SCP_string temp;
	va_list args;

	if (format == nullptr)
		return;

	va_start(args, format);
	vsprintf(temp, format, args);
	va_end(args);

	outwnd_print("General", temp.c_str());
}

void outwnd_printf(const char *id, const char *format, ...)
{
	SCP_string temp;
	va_list args;

	if ((id == nullptr) || (format == nullptr))
		return;

	va_start(args, format);
	vsprintf(temp, format, args);
	va_end(args);

	outwnd_print(id, temp.c_str());
}

void outwnd_print(const char *id, const char *tmp)
{
	if (running_unittests) {
		// Ignore all messages when running unit tests
		return;
	}

	if ((id == nullptr) || (tmp == nullptr))
		return;

	if (!outwnd_inited)
		return;

	if (Outwnd_no_filter_file == 1) {
		Outwnd_no_filter_file = 2;

		outwnd_print("general", "==========================================================================\n");
		outwnd_print("general", "DEBUG SPEW: No debug_filter.cfg found, so only general, error, warning and\n");
		outwnd_print("general", "scripting categories can be shown and no debug_filter.cfg info will be saved.\n");
		outwnd_print("general", "==========================================================================\n");
	}

	auto filter = std::find_if(filter_vector().begin(), filter_vector().end(), [&id] (const outwnd_filter_struct& f) { return stricmp(f.name, id) == 0; });

	// id found that isn't in the filter list yet
	if ( filter == filter_vector().end() ) {
		// Only create new filters if there was a filter file
		if (Outwnd_no_filter_file)
			return;

		Assert( strlen(id)+1 < NAME_LENGTH );
		outwnd_filter_struct new_filter;

		strcpy_s(new_filter.name, id);
		for (const char *name : FILTERS_ENABLED_BY_DEFAULT){
			new_filter.enabled = new_filter.enabled || stricmp(new_filter.name, name) == 0;
		}
		filter_vector().push_back( new_filter );
		save_filter_info();
	}
	else if (!filter->enabled)
			return;

	if (Log_debug_output_to_file) {
		if (Log_fp != nullptr) {
			fputs(tmp, Log_fp);
			fflush(Log_fp);
		}
	}

	if (debugWindow) {
		debugWindow->addDebugMessage(id, tmp);
	}
}

extern const char* Osapi_legacy_mode_reason; // This was not exported in a header since it's not intended for general use

void outwnd_init()
{
	if (outwnd_inited)
		return;

	if (!running_unittests && Log_fp == nullptr) {
		SCP_string logpath;
		if (!Cmdline_log_to_stdout) {
			char pathname[MAX_PATH_LEN];

			/* Set where the log file is going to go */
			// Zacam: Set various conditions based on what type of log to generate.
			if (Fred_running) {
				FreeSpace_logfilename = "fred2_open.log";
			} else if (Is_standalone) {
				FreeSpace_logfilename = "fs2_standalone.log";
			} else {
				FreeSpace_logfilename = "fs2_open.log";
			}

			// create data file path if it does not exist
			_mkdir(os_get_config_path(Pathtypes[CF_TYPE_DATA].path).c_str());

			memset(pathname, 0, sizeof(pathname));
			snprintf(pathname, MAX_PATH_LEN, "%s/%s", Pathtypes[CF_TYPE_DATA].path, FreeSpace_logfilename);

			logpath = os_get_config_path(pathname);

			Log_fp       = fopen(logpath.c_str(), "wb");
			Log_close_fp = true;
		} else {
			Log_fp       = stdout;
			Log_close_fp = false;

			logpath = "<stdout>";
		}

		outwnd_inited = Log_fp != nullptr;

		if (Log_fp == nullptr) {
			fprintf(stderr, "Error opening %s\n", logpath.c_str());
		} else {
			time_t timedate = time(nullptr);
			char datestr[50];

			memset(datestr, 0, sizeof(datestr));
			strftime(datestr, sizeof(datestr) - 1, "%a %b %d %H:%M:%S %Y", localtime(&timedate));

			outwnd_printf("General", "Opened log '%s', %s ...\n", logpath.c_str(), datestr);
			mprintf(("Legacy config mode is %s.\nReason: %s\n", os_is_legacy_mode() ? "ENABLED" : "DISABLED",
					 Osapi_legacy_mode_reason));
		}
	}
}

void outwnd_close()
{
	if (!running_unittests && Log_fp != nullptr) {
		time_t timedate = time(nullptr);
		char datestr[50];

		memset(datestr, 0, sizeof(datestr));
		strftime(datestr, sizeof(datestr) - 1, "%a %b %d %H:%M:%S %Y", localtime(&timedate));

		outwnd_printf("General", "... Log closed, %s\n", datestr);

		if (Log_close_fp) {
			fclose(Log_fp);
		}
		Log_fp       = nullptr;
		Log_close_fp = false;
	}

	outwnd_inited = false;
}

void outwnd_debug_window_init() {
	debugWindow.reset(new osapi::DebugWindow());
}
void outwnd_debug_window_do_frame(float frametime) {
	debugWindow->doFrame(frametime);
}
void outwnd_debug_window_deinit() {
	debugWindow.reset();
}
