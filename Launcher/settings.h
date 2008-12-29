#ifndef __STATIC_H__
#define __STATIC_H__

class Settings
{
public:
	static char exe_filepath[MAX_PATH];
	static char exe_pathonly[MAX_PATH];
	static char exe_nameonly[MAX_PATH];
	static bool exe_path_valid;

	static void set_exe_filepath(char *path);
	static void set_reg_path(char *path, int type);

	static bool get_mod_from_ini();
	static bool get_exe_path_from_ini();

	static char mod_name[MAX_PATH];
	static char reg_path[MAX_PATH];
	static int  exe_type;

	static char *ini_main;
	static char *ini_mod_default;
	static char *ini_mod_custom;
	static char *command_cfg;

	static char flags_custom[10240];
	static char flags_default_mod[10240];
	static char flags_custom_mod[10240];

	static void is_openal(bool value);
	static bool openal_build;
};

#endif