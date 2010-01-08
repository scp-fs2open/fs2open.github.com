#include "stdafx.h"
#include "mod_settings.h"
#include "launcher_settings.h"
#include "misc.h"

#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"


char ModSettings::m_mod_params[1024] = "";
char ModSettings::m_standard_params[1024] = "";
char ModSettings::m_custom_params[1024] = "";


bool ModSettings::load_mod()
{
	char ini_name[MAX_PATH];
	sprintf(ini_name, "%s\\%s\\%s", LauncherSettings::get_exe_pathonly(), LauncherSettings::get_active_mod(), const_cast<char*>(INI_MOD_DEFAULT));

	dictionary *ini = iniparser_load(ini_name);
	if (ini == NULL)
		return false;

	iniparser_dump(ini, stderr);

	// read ini variables
	// TODO
	Int3();

	// atomically assign all ini variables
	// TODO
	Int3();

	iniparser_freedict(ini);
	return true;
}

bool ModSettings::load_user()
{
	if (LauncherSettings::get_exe_type() == EXE_TYPE_CUSTOM)
		return load_user_fso();
	else
		return load_user_retail();
}

bool ModSettings::load_user_fso()
{
	char ini_name[MAX_PATH];
	sprintf(ini_name, "%s\\%s\\%s", LauncherSettings::get_exe_pathonly(), LauncherSettings::get_active_mod(), const_cast<char*>(INI_MOD_CUSTOM));

	dictionary *ini = iniparser_load(ini_name);
	if (ini == NULL)
		return false;

	iniparser_dump(ini, stderr);

	// read ini variables
	char *standard_params = iniparser_getstr(ini, "settings:flags");

	if (standard_params == NULL)
		return false;

	// atomically assign all ini variables
	strcpy(m_standard_params, standard_params);
	strcpy(m_custom_params, "");

	iniparser_freedict(ini);
	return true;
}

bool ModSettings::load_user_retail()
{
	FILE *fp = NULL;
	char path_buffer[MAX_PATH];

	bool exists = check_cfg_file(path_buffer);
	if (!exists)
		return false;

	fp = fopen(path_buffer, "rt");
	if (fp == NULL)
		return false;

	// read cfg variables
	char custom_params[MAX_CUSTOM_PARAM_SIZE];
	fgets(m_custom_params, sizeof(m_custom_params) - 1, fp);

	//if (custom_params == NULL)
	//	return false;

	// atomically assign all cfg variables
	strcpy(m_standard_params, "");
	strcpy(m_custom_params, custom_params);

	fclose(fp);
	return true;
}

bool ModSettings::save_user()
{
	char path_buffer[MAX_PATH];
	char all_params[3000] = { 0 };
	FILE *fp;

	// build command line (sans exe)
	if (*m_mod_params)
	{
		strcat(all_params, m_mod_params);
		strcat(all_params, " ");
	}
	if (*m_standard_params)
	{
		strcat(all_params, m_standard_params);
		strcat(all_params, " ");
	}
	if (*m_custom_params)
	{
		strcat(all_params, m_custom_params);
		strcat(all_params, " ");
	}
	trim(all_params);

	// write to config file
	check_cfg_file(path_buffer, true);
	fp = fopen(path_buffer, "wt");
	if (fp)
	{
		fwrite(all_params, len * sizeof(char), 1, fp);
		fclose(fp);
	}

	// write standard params to ini file
	char ini_name[MAX_PATH];
	sprintf(ini_name, "%s\\%s\\%s", LauncherSettings::get_exe_pathonly(), LauncherSettings::get_active_mod(), const_cast<char*>(INI_MOD_CUSTOM));

	fp = ini_open_for_write(ini_name, false, "These are the user's custom settings; don't distribute them with the mod because he'll want to choose his own");
	if (fp == NULL)
		return false;

	ini_write_type(fp, "[settings]");
	ini_write_data(fp, "flags", m_standard_params);

	ini_close(fp);
	return true;
}

char *ModSettings::get_mod_parameters()
{
	return m_mod_params;
}

char *ModSettings::get_standard_parameters()
{
	return m_standard_params;
}

char *ModSettings::get_custom_parameters()
{
	return m_custom_params;
}

void ModSettings::set_standard_parameters(const char *standard_params)
{
	strcpy(m_standard_params, standard_params);
}

void ModSettings::set_custom_parameters(const char *custom_params)
{
	strcpy(m_custom_params, custom_params);
}
